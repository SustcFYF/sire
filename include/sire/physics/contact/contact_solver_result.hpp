#ifndef SIRE_CONTACT_SOLVER_RESULT_HPP_
#define SIRE_CONTACT_SOLVER_RESULT_HPP_
#include <algorithm>
#include <vector>

#include <hpp/fcl/data_types.h>

#include <aris/core/basic_type.hpp>
#include <aris/core/object.hpp>

#include "sire/core/constants.hpp"
#include "sire/core/geometry/geometry_base.hpp"

namespace sire::physics::contact {
using namespace hpp;
struct ContactSolverResult {
  void resize(int num_velocities, int num_contacts) {
    vs_next.resize(num_velocities, 0);
    fn.resize(num_contacts, 0);
    ft.resize(2 * num_contacts, 0);
    vn.resize(num_contacts, 0);
    vt.resize(2 * num_contacts, 0);
  }
  // ��һʱ�̵��ٶ�����
  std::vector<double> vs_next;
  // ����Ӵ���
  std::vector<double> fn;
  // ����Ӵ���
  std::vector<double> ft;
  // ����Ӵ��ٶȣ�������Ҳ���Բ��ã�
  std::vector<double> vn;
  // ����Ӵ��ٶȣ�������Ҳ���Բ��ã�
  std::vector<double> vt;
};
}  // namespace sire::physics::contact
#endif