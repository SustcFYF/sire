#ifndef SIRE_RECORDER_HPP_
#define SIRE_RECORDER_HPP_
#include <sire_lib_export.h>

#include <aris/core/object.hpp>

namespace sire::simulator {
/**
* 1. �ٶ�ֻ��Model�е�������Ҫ����¼��֧�ַ���طŹ���
* 2. �ٶ�Model��ֻ��ForcePool�Ľṹ�ᷢ���仯��
* 3. �ٶ��˶������У������˱���ṹ���ᷢ���仯���������ᷢ���仯
* 
* ��ʵ��Ҫ����������Model��CopyConstructor���޶��ʣ�
* ���Կ���дһ����������Modelָ��ķ���src���Ƶ�dest��Ȼ��dest init��
*/
class Record {
 public:

};

class Recorder : aris::core::NamedObject {
 public:
  auto registerRecord() -> void;
   auto virtual init() -> void{};
  Recorder() = default;
  virtual ~Recorder() = default;
  
 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
};
}  // namespace sire::simulator
#endif