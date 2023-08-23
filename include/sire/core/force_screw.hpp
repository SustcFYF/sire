#ifndef SIRE_FORCE_SCREW_HPP_
#define SIRE_FORCE_SCREW_HPP_
#include <cmath>
#include <sire_lib_export.h>
namespace sire::core::screw {
// ���Ŷ���
// f: 3x1 ����
// pe: 6x1 λ����ŷ���Ǳ�ʾ�ĳ���
// tau: 3x1 ����ż
// fs:  6x1 ������ [f, tau]
auto SIRE_API s_fpm2fs(const double* f, const double* pm, double* fs_out) -> void;
}
#endif