#include "sire/controller/controller_sensor.hpp"

#include "aris/core/pipe.hpp"
#include "aris/core/reflection.hpp"

namespace sire::controller {
struct NrtSensor::Imp {
  std::atomic_bool is_stopping_;
  std::thread sensor_thread_;
  std::atomic_int data_to_read_;
  std::recursive_mutex data_mutex_[3];
  std::unique_ptr<aris::control::SensorData> data_[3];

  static auto update_thread(NrtSensor* sensor) -> void {
    std::unique_lock<std::recursive_mutex> lock_(sensor->imp_->data_mutex_[0]);

    while (!sensor->imp_->is_stopping_) {
      for (int i = 0; i < 3; ++i) {
        sensor->imp_->data_to_read_ = (i + 2) % 3;
        sensor->updateData(std::ref(*sensor->imp_->data_[i]));
        std::unique_lock<std::recursive_mutex> nextLock(
            sensor->imp_->data_mutex_[(i + 1) % 3]);
        lock_.swap(nextLock);
      }
    }
  };
};
auto NrtSensor::copiedDataPtr() -> std::unique_ptr<aris::control::SensorData> {
  return std::make_unique<aris::control::SensorData>(dataProtector().data());
}
auto NrtSensor::dataProtector() -> NrtSensorDataProtector {
  return std::move(NrtSensorDataProtector(this));
}
auto NrtSensor::init() -> void{};
auto NrtSensor::start() -> void {
  if (!imp_->sensor_thread_.joinable()) {
    init();

    std::lock(imp_->data_mutex_[0], imp_->data_mutex_[1], imp_->data_mutex_[2]);

    std::unique_lock<std::recursive_mutex> lock1(imp_->data_mutex_[0],
                                                 std::adopt_lock);
    std::unique_lock<std::recursive_mutex> lock2(imp_->data_mutex_[1],
                                                 std::adopt_lock);
    std::unique_lock<std::recursive_mutex> lock3(imp_->data_mutex_[2],
                                                 std::adopt_lock);

    for (auto& d : imp_->data_) {
      this->updateData(std::ref(*d));
    }

    imp_->is_stopping_ = false;
    imp_->data_to_read_ = 2;

    imp_->sensor_thread_ = std::thread(Imp::update_thread, this);
  }
};

auto NrtSensor::stop() -> void {
  if (imp_->sensor_thread_.joinable()) {
    imp_->is_stopping_ = true;
    imp_->sensor_thread_.join();
    release();
  }
};
NrtSensor::~NrtSensor() = default;
NrtSensor::NrtSensor(
    std::function<aris::control::SensorData*()> sensor_data_ctor,
    const std::string& name, const std::string& desc, bool is_virtual,
    bool activate, sire::Size frequency)
    : SensorBase(sensor_data_ctor, name, desc, is_virtual, activate, frequency),
      imp_(new Imp) {}

NrtSensorDataProtector::NrtSensorDataProtector(NrtSensor* nrt_sensor)
    : nrt_sensor_(nrt_sensor), data_(nullptr) {
  // ����data_to_read_ָ�����µ��ڴ�,�������data_to_read_Ϊ2,��ô�����������
  //  1.��ʱ����д�ڴ�0,�ڴ�1���С�
  //  2.��ĳЩ��������ʱ����,sensor���øո�д���ڴ�1,��׼���ͷ�dataMutex0,����֮��׼����data_to_read_��Ϊ0��
  //    ���������������,dataMutex2���ᱻ��ס��
  // �����������������,�̶��ᷢ�����������
  //  1.����д�ڴ�0,�ڴ�1����,dataMutex2���ᱻ��ס��data_to_read_��ȻΪ2,��ô�˺�����һֱ�ڲ����ڴ�2,��ȫ��
  //  2.dataMutex2����ס��ͬʱ,data_to_read_�����µ�0,��ʱ��������ʼд�ڴ�1,����dataMutex2����,��˴�����һֱ�޷�
  //    ���µ��ڴ�2���������ݶ�ȡ�����ڴ�0,��ȫ��

  do {
    lock_ = std::unique_lock<std::recursive_mutex>(
        nrt_sensor_->imp_->data_mutex_[nrt_sensor->imp_->data_to_read_],
        std::try_to_lock);
  } while (!lock_.owns_lock());

  data_ = nrt_sensor_->imp_->data_[nrt_sensor->imp_->data_to_read_].get();
};

RtSensor::~RtSensor() = default;
RtSensor::RtSensor(std::function<aris::control::SensorData*()> sensor_data_ctor,
                   const std::string& name, const std::string& desc,
                   bool is_virtual, bool activate, sire::Size frequency)
    : SensorBase(sensor_data_ctor, name, desc, is_virtual, activate,
                 frequency) {}

struct SensorDataBuffer::Imp {
  sire::Size buffer_size_{50};
  std::atomic_bool buffer_is_full_{false};
  std::atomic_int data_to_read{-1};
  std::atomic_int data_to_write{0};
  std::recursive_mutex data_to_read_mutex_;
  std::recursive_mutex data_to_write_mutex_;
  std::vector<std::unique_ptr<aris::control::SensorData>> buffer_;
  std::vector<std::recursive_mutex> buffer_mutex_;
  ~Imp() = default;
  Imp(sire::Size buffer_size = 50)
      : buffer_size_(buffer_size),
        buffer_(buffer_size),
        buffer_mutex_(buffer_size) {}
};
auto SensorDataBuffer::setBufferSize(sire::Size buffer_size) -> void {
  imp_->buffer_size_ = buffer_size;
  std::vector<std::recursive_mutex> new_mutex_list(imp_->buffer_size_);
  imp_->buffer_mutex_.swap(new_mutex_list);
  imp_->buffer_.resize(imp_->buffer_size_);
}
auto SensorDataBuffer::bufferSize() const -> sire::Size {
  return imp_->buffer_size_;
}
auto SensorDataBuffer::updateBufferData(
    std::unique_ptr<aris::control::SensorData> data) -> void {
  std::unique_lock<std::recursive_mutex> data_to_read_lock(
      imp_->data_to_read_mutex_, std::defer_lock);
  std::unique_lock<std::recursive_mutex> data_to_write_lock(
      imp_->data_to_write_mutex_, std::defer_lock);
  std::lock(data_to_read_lock, data_to_write_lock);
  std::unique_lock<std::recursive_mutex> lock_data_write(
      imp_->buffer_mutex_[imp_->data_to_write]);
  if (imp_->buffer_is_full_) {
    imp_->buffer_[imp_->data_to_write] = std::move(data);
    imp_->data_to_read = (imp_->data_to_read + 1) % imp_->buffer_size_;
    imp_->data_to_write = (imp_->data_to_write + 1) % imp_->buffer_size_;
  } else {
    imp_->buffer_[imp_->data_to_write] = std::move(data);
    imp_->data_to_write = (imp_->data_to_write + 1) % imp_->buffer_size_;
    if (imp_->data_to_read == imp_->data_to_write) {
      imp_->buffer_is_full_ = true;
    }
    if (imp_->data_to_read == -1) {
      imp_->data_to_read = 0;
    }
  }
};
auto SensorDataBuffer::retrieveBufferData(
    std::vector<std::unique_ptr<aris::control::SensorData>>& vec,
    sire::Size& count) -> void {
  for (int i = 0; i < vec.size(); ++i) {
    // ��������ķ�ʽ������
    std::unique_lock<std::recursive_mutex> data_to_read_lock(
        imp_->data_to_read_mutex_, std::defer_lock);
    std::unique_lock<std::recursive_mutex> data_to_write_lock(
        imp_->data_to_write_mutex_, std::defer_lock);
    std::lock(data_to_read_lock, data_to_write_lock);
    if (imp_->data_to_read == -1) {
      return;
    }
    std::unique_lock<std::recursive_mutex> lock_data_read(
        imp_->buffer_mutex_[imp_->data_to_read]);
    if (imp_->buffer_is_full_ || imp_->data_to_read != imp_->data_to_write) {
      vec[i] = std::move(imp_->buffer_[imp_->data_to_read]);
      imp_->data_to_read = (imp_->data_to_read + 1) % imp_->buffer_size_;
      imp_->buffer_is_full_ = false;
      ++count;
    } else {
      break;
    }
  }
}
auto SensorDataBuffer::retrieveData()
    -> std::unique_ptr<aris::control::SensorData> {
  std::unique_lock<std::recursive_mutex> data_to_read_lock(
      imp_->data_to_read_mutex_, std::defer_lock);
  std::unique_lock<std::recursive_mutex> data_to_write_lock(
      imp_->data_to_write_mutex_, std::defer_lock);
  std::lock(data_to_read_lock, data_to_write_lock);
  std::unique_lock<std::recursive_mutex> lock_data_read(
      imp_->buffer_mutex_[(imp_->data_to_read + imp_->buffer_size_ - 1) %
                          imp_->buffer_size_]);
  if (imp_->data_to_read != imp_->data_to_write) {
    std::unique_ptr<aris::control::SensorData> ptr(
        new aris::control::SensorData{
            *imp_->buffer_[imp_->data_to_write - sire::Size(1)]});
    return std::move(ptr);
  }
  return nullptr;
}
SensorDataBuffer::~SensorDataBuffer() = default;
SensorDataBuffer::SensorDataBuffer(sire::Size buffer_size)
    : imp_(new Imp(buffer_size)) {}

struct BufferedRtSensor::Imp {
  std::unique_ptr<SensorDataBuffer> buffer_;
  sire::Size count_{0};
  std::thread update_buffer_data_thread_;
  std::atomic_bool is_update_buffer_data_thread_running_;
  aris::core::Pipe data_pipe_;
};
auto BufferedRtSensor::setBufferSize(sire::Size buffer_size) -> void {
  imp_->buffer_->setBufferSize(buffer_size);
}
auto BufferedRtSensor::bufferSize() const -> sire::Size {
  return imp_->buffer_->bufferSize();
}
auto BufferedRtSensor::updateBufferData(
    std::unique_ptr<aris::control::SensorData> data) -> void {
  imp_->buffer_->updateBufferData(std::move(data));
}
auto BufferedRtSensor::retrieveBufferData(
    std::vector<std::unique_ptr<aris::control::SensorData>>& vec,
    sire::Size& count) -> void {
  imp_->buffer_->retrieveBufferData(vec, count);
}
auto BufferedRtSensor::retrieveData()
    -> std::unique_ptr<aris::control::SensorData> {
  return imp_->buffer_->retrieveData();
}
auto BufferedRtSensor::updateData(
    std::unique_ptr<aris::control::SensorData> data) -> void {
  if (frequency() == 0) {
    updateBufferData(std::move(data));
  } else {
    if (imp_->count_ % (1000 / frequency()) == 0) {
      updateBufferData(std::move(data));
    }
    imp_->count_ = (imp_->count_ + 1) % (1000 / frequency());
  }
}
// TODO: add const quantifier
auto BufferedRtSensor::lockFreeUpdateData(aris::control::SensorData& data)
    -> void {
  aris::core::MsgFix<MAX_MSG_SIZE> msg;
  std::string str;
  data.to_json_string(str);
  msg.copy(str);
  imp_->data_pipe_.sendMsg(msg);
}
auto BufferedRtSensor::start() -> void {
  imp_->is_update_buffer_data_thread_running_.store(true);
  imp_->update_buffer_data_thread_ = std::thread([this]() {
    aris::core::Msg msg;
    while (imp_->is_update_buffer_data_thread_running_) {
      if (imp_->data_pipe_.recvMsg(msg)) {
        if (!msg.empty()) {
          aris::control::SensorData data;
          try {
            data.from_json_string(msg.toString());
            updateData(std::make_unique<aris::control::SensorData>(data));
          } catch (nlohmann::json::parse_error& err) {
            continue;
            // std::cout << "not json and skip with msg: " << msg.toString()
            //           << std::endl;
          }
        }
      }
    }
  });
}
auto BufferedRtSensor::stop() -> void {
  imp_->is_update_buffer_data_thread_running_.store(false);
  imp_->update_buffer_data_thread_.join();
}
BufferedRtSensor::~BufferedRtSensor() = default;
BufferedRtSensor::BufferedRtSensor(
    std::function<aris::control::SensorData*()> sensor_data_ctor,
    const std::string& name, const std::string& desc, bool is_virtual,
    bool activate, sire::Size frequency, sire::Size buffer_size)
    : RtSensor(sensor_data_ctor, name, desc, is_virtual, activate, frequency),
      imp_(new Imp) {
  imp_->buffer_.reset(new SensorDataBuffer(buffer_size));
};

auto MotorForceData::to_json_string(std::string& str) -> void {
  nlohmann::json j;
  to_json(j, *this);
  str = j.dump();
};
auto MotorForceData::from_json_string(const std::string& str) -> void {
  from_json(nlohmann::json::parse(str), *this);
};

struct BufferedMotorForceVirtualSensor::Imp {
  sire::Size motor_index_{0};
  Imp() : motor_index_(0) {}
};
auto BufferedMotorForceVirtualSensor::motorIndex() const -> sire::Size {
  return imp_->motor_index_;
}
auto BufferedMotorForceVirtualSensor::setMotorIndex(sire::Size index) -> void {
  imp_->motor_index_ = index;
}
auto BufferedMotorForceVirtualSensor::init() -> void {}
auto BufferedMotorForceVirtualSensor::start() -> void {
  BufferedRtSensor::start();
}
auto BufferedMotorForceVirtualSensor::stop() -> void {}
auto BufferedMotorForceVirtualSensor::updateData(
    std::unique_ptr<aris::control::SensorData> data) -> void {
  BufferedRtSensor::updateData(std::move(data));
}
auto BufferedMotorForceVirtualSensor::copiedDataPtr()
    -> std::unique_ptr<aris::control::SensorData> {
  return std::move(retrieveData());
}
auto BufferedMotorForceVirtualSensor::getRtData(aris::control::SensorData* data)
    -> void{};
BufferedMotorForceVirtualSensor::~BufferedMotorForceVirtualSensor() = default;
BufferedMotorForceVirtualSensor::BufferedMotorForceVirtualSensor(
    sire::Size motor_index)
    : BufferedRtSensorTemplate<MotorForceData>(), imp_(new Imp) {
  imp_->motor_index_ = motor_index;
}

struct MotorForceVirtualSensor::Imp {
  sire::Size motor_index_{0};
  sire::Size count_{0};
  std::unique_ptr<aris::control::SensorData> data_{
      std::make_unique<controller::MotorForceData>(0)};
  Imp()
      : motor_index_(0),
        count_(0),
        data_(std::make_unique<controller::MotorForceData>(0)) {}
};
auto MotorForceVirtualSensor::motorIndex() const -> sire::Size {
  return imp_->motor_index_;
}
auto MotorForceVirtualSensor::setMotorIndex(sire::Size index) -> void {
  imp_->motor_index_ = index;
}
auto MotorForceVirtualSensor::init() -> void {}
auto MotorForceVirtualSensor::start() -> void {}
auto MotorForceVirtualSensor::stop() -> void {}
auto MotorForceVirtualSensor::updateData(
    std::unique_ptr<aris::control::SensorData> data) -> void {
  if (frequency() == 0) {
    imp_->data_ = std::move(data);
  } else {
    if (imp_->count_ % (1000 / frequency()) == 0) {
      imp_->data_ = std::move(data);
    }
    imp_->count_ = (imp_->count_ + 1) % (1000 / frequency());
  }
}
auto MotorForceVirtualSensor::copiedDataPtr()
    -> std::unique_ptr<aris::control::SensorData> {
  return std::move(imp_->data_);
}
auto MotorForceVirtualSensor::getRtData(aris::control::SensorData* data)
    -> void{};
MotorForceVirtualSensor::~MotorForceVirtualSensor() = default;
MotorForceVirtualSensor::MotorForceVirtualSensor(sire::Size motor_index)
    : RtSensorTemplate<MotorForceData>(), imp_(new Imp) {
  imp_->motor_index_ = motor_index;
}

ARIS_REGISTRATION {
  aris::core::class_<NrtSensor>("NrtSensor")
      .inherit<aris::control::SensorBase>();
  aris::core::class_<RtSensor>("RtSensor").inherit<aris::control::SensorBase>();
  aris::core::class_<BufferedRtSensor>("BufferedRtSensor")
      .inherit<RtSensor>()
      .prop("buffer_size", &BufferedRtSensor::setBufferSize,
            &BufferedRtSensor::bufferSize);
  aris::core::class_<BufferedMotorForceVirtualSensor>(
      "BufferedMotorForceControllerVirtualSensor")
      .inherit<BufferedRtSensor>()
      .prop("motor_index", &BufferedMotorForceVirtualSensor::setMotorIndex,
            &BufferedMotorForceVirtualSensor::motorIndex);
  aris::core::class_<MotorForceVirtualSensor>(
      "MotorForceControllerVirtualSensor")
      .inherit<RtSensor>()
      .prop("motor_index", &MotorForceVirtualSensor::setMotorIndex,
            &MotorForceVirtualSensor::motorIndex);
}
};  // namespace sire::controller
