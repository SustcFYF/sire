#include "sire/physics/geometry/sphere_collision_geometry.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include <hpp/fcl/bvh/BVH_model.h>
#include <hpp/fcl/shape/geometric_shapes.h>

#include <aris/core/reflection.hpp>

namespace sire::physics::geometry {
SIRE_DEFINE_TO_JSON_HEAD(SphereCollisionGeometry) {
  j = json{{"shape_type", shapeType()}, {"radius", radius()}};
}

SIRE_DEFINE_FROM_JSON_HEAD(SphereCollisionGeometry) {
  j.at("shape_type").get_to(shapeType());
  j.at("radius").get_to(radius());
}
auto SphereCollisionGeometry::init() -> void {
  fcl::Transform3f trans(
      fcl::Matrix3f{{partPm()[0][0], partPm()[0][1], partPm()[0][2]},
                    {partPm()[1][0], partPm()[1][1], partPm()[1][2]},
                    {partPm()[2][0], partPm()[2][1], partPm()[2][2]}},
      fcl::Vec3f{partPm()[0][3], partPm()[1][3], partPm()[2][3]});
  resetCollisionObject(
      new fcl::CollisionObject(make_shared<fcl::Sphere>(radius()), trans));
}
SphereCollisionGeometry::SphereCollisionGeometry(double radius,
                                                 const double* prt_pm)
    : CollidableGeometry(prt_pm), SphereShape(radius) {}
SphereCollisionGeometry::~SphereCollisionGeometry() = default;

// �������ڲ���from_json to_json���壬
// ʹ�ú궨���������json����ת����from_json to_json�ķ�������
SIRE_DEFINE_JSON_OUTER_TWO(SphereCollisionGeometry)

ARIS_REGISTRATION {
  aris::core::class_<SphereCollisionGeometry>("SphereCollisionGeometry")
      .inherit<CollidableGeometry>()
      .inherit<sire::core::geometry::SphereShape>();
}
}  // namespace sire::physics::geometry