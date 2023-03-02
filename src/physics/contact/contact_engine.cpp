#include "sire/physics/contact/contact_engine.hpp"

#include <stdio.h>

#include <fstream>
#include <mutex>
#include <string>
#include <thread>

#include <hpp/fcl/broadphase/broadphase_dynamic_AABB_tree.h>
#include <hpp/fcl/distance.h>
#include <hpp/fcl/math/transform.h>
#include <hpp/fcl/mesh_loader/assimp.h>
#include <hpp/fcl/mesh_loader/loader.h>
#include <hpp/fcl/shape/geometric_shapes.h>

#include <aris/core/reflection.hpp>
#include <aris/server/control_server.hpp>

#include "sire/core/constants.hpp"
#include "sire/physics/collision/collision_detection_engine.hpp"
#include "sire/physics/collision/collision_exists_callback.hpp"
#include "sire/transfer/part_pq_transfer.hpp"
namespace sire::physics::contact {
struct ContactEngine::Imp {
  unique_ptr<aris::core::PointerArray<geometry::CollidableGeometry,
                                      aris::dynamic::Geometry>>
      dynamic_geometry_pool_;
  unique_ptr<aris::core::PointerArray<geometry::CollidableGeometry,
                                      aris::dynamic::Geometry>>
      anchored_geometry_pool_;
  fcl::DynamicAABBTreeCollisionManager dynamic_tree_;
  unordered_map<core::geometry::GeometryId, geometry::CollidableGeometry*>
      dynamic_objects_map_;
  fcl::DynamicAABBTreeCollisionManager anchored_tree_;
  unordered_map<core::geometry::GeometryId, geometry::CollidableGeometry*>
      anchored_objects_map_;
  unique_ptr<collision::CollisionFilter> collision_filter_;

  aris::server::ControlServer* server_;
  aris::core::PointerArray<aris::dynamic::Part, aris::dynamic::Element>*
      part_pool_ptr_;
  sire::Size part_size_;
};

auto ContactEngine::resetCollisionFilter(collision::CollisionFilter* filter)
    -> void {
  imp_->collision_filter_.reset(filter);
}
auto ContactEngine::collisionFilter() -> collision::CollisionFilter& {
  return *imp_->collision_filter_;
}
auto ContactEngine::resetDynamicGeometryPool(
    aris::core::PointerArray<geometry::CollidableGeometry,
                             aris::dynamic::Geometry>* pool) -> void {
  imp_->dynamic_geometry_pool_.reset(pool);
}
auto ContactEngine::dynamicGeometryPool()
    -> aris::core::PointerArray<geometry::CollidableGeometry,
                                aris::dynamic::Geometry>& {
  return *imp_->dynamic_geometry_pool_;
}
auto ContactEngine::resetAnchoredGeometryPool(
    aris::core::PointerArray<geometry::CollidableGeometry,
                             aris::dynamic::Geometry>* pool) -> void {
  imp_->anchored_geometry_pool_.reset(pool);
}
auto ContactEngine::anchoredGeometryPool()
    -> aris::core::PointerArray<geometry::CollidableGeometry,
                                aris::dynamic::Geometry>& {
  return *imp_->anchored_geometry_pool_;
}
auto ContactEngine::addDynamicGeometry(
    geometry::CollidableGeometry& dynamic_geometry) -> bool {
  dynamic_geometry.updateLocation(nullptr);
  dynamic_geometry.getCollisionObject()->computeAABB();
  imp_->dynamic_tree_.registerObject(dynamic_geometry.getCollisionObject());
  imp_->dynamic_tree_.update();
  imp_->dynamic_objects_map_[dynamic_geometry.geometryId()] = &dynamic_geometry;
  return true;
}
auto ContactEngine::addAnchoredGeometry(
    geometry::CollidableGeometry& anchored_geometry) -> bool {
  anchored_geometry.updateLocation(nullptr);
  anchored_geometry.getCollisionObject()->computeAABB();
  imp_->anchored_tree_.registerObject(anchored_geometry.getCollisionObject());
  imp_->anchored_tree_.update();
  imp_->anchored_objects_map_[anchored_geometry.geometryId()] =
      &anchored_geometry;
  return false;
}
auto ContactEngine::removeGeometry() -> bool { return false; }
auto ContactEngine::clearDynamicGeometry() -> bool {
  imp_->dynamic_tree_.clear();
  return true;
}
auto ContactEngine::clearAnchoredGeometry() -> bool {
  imp_->anchored_tree_.clear();
  return true;
}

auto ContactEngine::init() -> void {
  for (auto& anchored_geometry : *imp_->anchored_geometry_pool_) {
    anchored_geometry.init();
    addAnchoredGeometry(anchored_geometry);
    imp_->collision_filter_->addGeometry(anchored_geometry);
  }
  for (auto& dynamic_geometry : *imp_->dynamic_geometry_pool_) {
    dynamic_geometry.init();
    addDynamicGeometry(dynamic_geometry);
    imp_->collision_filter_->addGeometry(dynamic_geometry);
  }
  imp_->collision_filter_->loadMatConfig();
  imp_->server_ = &aris::server::ControlServer::instance();
  imp_->part_pool_ptr_ =
      &dynamic_cast<aris::dynamic::Model*>(&imp_->server_->model())->partPool();
  imp_->part_size_ = imp_->part_pool_ptr_->size();
}
// callback print
// cout << "has collision number: " <<
// callback.data.result.numContacts() << endl;
// if (callback.collidedObjectMap().size() != 0) {
//   cout << callback.collidedObjectMap().size() << endl;
// }
// for (auto& obj_pair : callback.collidedObjectMap()) {
//   cout << "collided object of "
//        // << imp_->collision_filter_->queryGeometryIdByPtr(c.o1) << "
//        "
//        << obj_pair.first
//        << " "
//        //<< imp_->collision_filter_->queryGeometryIdByPtr(c.o2) <<
//        endl;
//        << obj_pair.second << endl;
// }
ContactEngine::ContactEngine() : imp_(new Imp) {}
ContactEngine::~ContactEngine(){};

ARIS_REGISTRATION {
  typedef aris::core::PointerArray<geometry::CollidableGeometry,
                                   aris::dynamic::Geometry>& (
      ContactEngine::*GeometryPoolFunc)();
  typedef sire::physics::collision::CollisionFilter& (
      ContactEngine::*CollisionFilterPoolFunc)();
  aris::core::class_<ContactEngine>("ContactEngine")
      .prop("dynamic_geometry_pool", &ContactEngine::resetDynamicGeometryPool,
            GeometryPoolFunc(&ContactEngine::dynamicGeometryPool))
      .prop("anchored_geometry_pool", &ContactEngine::resetAnchoredGeometryPool,
            GeometryPoolFunc(&ContactEngine::anchoredGeometryPool))
      .prop("collision_filter", &ContactEngine::resetCollisionFilter,
            CollisionFilterPoolFunc(&ContactEngine::collisionFilter));
}
}  // namespace sire::contact