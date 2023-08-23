#ifndef SIRE_INTEGRATOR_BASE_HPP_
#define SIRE_INTEGRATOR_BASE_HPP_

#include <sire_lib_export.h>

#include <aris/core/object.hpp>
#include <aris/dynamic/model.hpp>

#include "sire/core/constants.hpp"

namespace sire {
namespace physics {
class PhysicsEngine;
}
namespace simulator {
class SIRE_API IntegratorBase {
 public:
  auto virtual integrate(double** diff_data_in, double* old_result,
                         double* result_out) -> bool = 0;
  auto init(physics::PhysicsEngine* engine) -> void;
  auto step(double dt) -> bool;

  /**
   * �����������д���������ʵ����Ҫ�ĳ�ʼ���������������init()�����б�����
   * Ĭ�ϵ�doInit()Ĭ��û�����κδ���
   */
  auto virtual doInit() -> void{};

  /**
  * TODO(leitianjian):
  *   ���Դ���Model��PhysicsEngine��ָ�룬�������ڳ�ʼ��ʱȷ��ָ�룬
  *   �бȽ�ǿ�Ŀ���չ�ԡ�
  * ���������ʵ�����������ʵ�����������Ҫ������������
  * 1. ����ϵͳ����״̬��p, v, a����״̬����dtʱ��
  * 2. ������������صĲ���������еĻ�
  * @param dt ���ֲ���ʱ��
  * @returns ����ɹ�������`true`�������Ϊ����ԭ��ʧ�ܣ�û�а취��״̬����
             һ��ʱ��dt���ͷ���`false`������Ϊ���̫�������ʧ��
  * @post �����ǰʱ����t���������`true`����ʱ��ᱻǰ����t+dt���������
  *       `false`ʱ��������t
  */
  auto virtual doStep(double dt) -> bool = 0;
  auto stepSize() const -> double;
  auto setStepSize(double step_size) -> void;
  auto dataLength() const -> sire::Size;
  auto setDataLength(sire::Size data_length) -> void;
  IntegratorBase();
  virtual ~IntegratorBase();
  ARIS_DECLARE_BIG_FOUR(IntegratorBase);

 protected:
  // Model������Ϣ
  aris::dynamic::Model* model_ptr_{nullptr};
  sire::Size part_pool_length_;
  sire::Size motion_pool_length_;
  sire::Size general_motion_pool_length_;

 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
};
}  // namespace simulator
}  // namespace sire
#endif