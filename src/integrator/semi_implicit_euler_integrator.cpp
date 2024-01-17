#include "sire/integrator/semi_implicit_euler_integrator.hpp"

#include <aris/core/serialization.hpp>
#include <aris/dynamic/model.hpp>

#include "sire/core/constants.hpp"
#include "sire/core/sire_assert.hpp"
#include "sire/integrator/integrator_base.hpp"

namespace sire::simulator {
SemiImplicitEulerIntegrator::SemiImplicitEulerIntegrator() : IntegratorBase(){};
auto SemiImplicitEulerIntegrator::doStep(double dt) -> bool {
  SIRE_ASSERT(model_ptr_ != nullptr);
  SIRE_ASSERT(dt > 0.0);
  if (model_ptr_->forwardDynamics()) {
    std::cout << "forward dynamic failed" << std::endl;
    return false;
  }
  // ����ÿ��Part����as���ֵ�vs֮����ֵ�ps�������û�ȥ
  double as_buffer[6]{0}, vs_buffer[6]{0}, pm_buffer[16]{0}, ps_buffer[6]{0};

  for (sire::Size i = 0; i < part_pool_length_; ++i) {
    auto& part = model_ptr_->partPool()[i];
    part.getAs(as_buffer);
    part.getVs(vs_buffer);
    part.getPm(pm_buffer);
    // aris::dynamic::dsp(1, 6, as_buffer);
    double temp_pm[16]{0}, pm_result[16]{0};
    for (sire::Size j = 0; j < kTwistSize; ++j) {
      vs_buffer[j] += dt * as_buffer[j];
      ps_buffer[j] = dt * vs_buffer[j];
    }
    aris::dynamic::s_ps2pm(ps_buffer, temp_pm);
    aris::dynamic::s_pm_dot_pm(temp_pm, pm_buffer, pm_result);
    part.setVs(vs_buffer);
    part.setPm(pm_result);
  }

  // ���ݸ��µĸ˼���ص���Ϣ����Motion��ֵ
  for (std::size_t i = 0; i < motion_pool_length_; ++i) {
    auto& motion = model_ptr_->motionPool().at(i);
    motion.updA();
    motion.updV();
    motion.updP();
  }
  // ������˼���ص�marker������˼�λ�ˣ���С���ˣ�
  model_ptr_->forwardKinematics();
  for (std::size_t i = 0; i < general_motion_pool_length_; ++i) {
    auto& general_motion = model_ptr_->generalMotionPool().at(i);
    general_motion.updA();
    general_motion.updV();
    general_motion.updP();
  }
  return true;
};
auto SemiImplicitEulerIntegrator::integrate(double** diff_data_in,
                                        double* old_result, double* result_out)
    -> bool {
  for (int i = 0; i < dataLength(); ++i) {
    result_out[i] = old_result[i] + stepSize() * diff_data_in[0][i];
  }
  return true;
};
ARIS_DEFINE_BIG_FOUR_CPP(SemiImplicitEulerIntegrator);

ARIS_REGISTRATION {
  aris::core::class_<SemiImplicitEulerIntegrator>("SemiImplicitEulerIntegrator")
      .inherit<IntegratorBase>();
}
}  // namespace sire::simulator