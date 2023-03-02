#include "sire/core/geometry/sphere_geometry.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include <aris/core/reflection.hpp>
#include <aris/dynamic/model.hpp>
#include <aris/server/control_server.hpp>

namespace sire::core::geometry {
SIRE_DEFINE_TO_JSON_HEAD(SphereGeometry) {
  j = json{{"shape_type", shapeType()}, {"radius", radius()}};
}

SIRE_DEFINE_FROM_JSON_HEAD(SphereGeometry) {
  j.at("shape_type").get_to(shapeType());
  j.at("radius").get_to(radius());
}

SphereGeometry::SphereGeometry(double radius, const double* prt_pm)
    : GeometryOnPart(), SphereShape(radius) {}

SphereGeometry::~SphereGeometry() = default;

ARIS_DEFINE_BIG_FOUR_CPP(SphereGeometry)

// �������ڲ���from_json to_json���壬
// ʹ�ú궨���������json����ת����from_json to_json�ķ�������
SIRE_DEFINE_JSON_OUTER_TWO(SphereGeometry)

ARIS_REGISTRATION {
  aris::core::class_<SphereGeometry>("SphereGeometry")
      .inherit<GeometryOnPart>()
      .inherit<SphereShape>();
}
}  // namespace sire::geometry