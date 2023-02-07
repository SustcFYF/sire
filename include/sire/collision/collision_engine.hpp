#ifndef COLLISION_ENGINE_H
#define COLLISION_ENGINE_H

#include "sire/geometry/geometry.hpp"
#include "sire/collision/collision_filter.hpp"
#include "sire/collision/collision_exists_callback.hpp"
#include <aris/core/expression_calculator.hpp>
#include <hpp/fcl/broadphase/broadphase_callbacks.h>
#include <hpp/fcl/broadphase/broadphase_collision_manager.h>
#include <hpp/fcl/broadphase/default_broadphase_callbacks.h>
#include <hpp/fcl/collision.h>
#include <hpp/fcl/collision_data.h>
#include <hpp/fcl/collision_object.h>
#include <map>
#include <string>

namespace sire::collision {
using namespace std;
using namespace hpp;
// drake-based implementation
// filter和geometry配置都先读进去，之后通过init进行碰撞管理器的初始化
class SIRE_API CollisionEngine {
 public:
  auto resetCollisionFilter(CollisionFilter* filter) -> void;
  auto collisionFilter() -> CollisionFilter&;
  auto resetDynamicGeometryPool(
      aris::core::PointerArray<geometry::CollisionGeometry,
                               aris::dynamic::Geometry>* pool) -> void;
  auto dynamicGeometryPool()
      -> aris::core::PointerArray<geometry::CollisionGeometry,
                                  aris::dynamic::Geometry>&;
  auto resetAnchoredGeometryPool(
      aris::core::PointerArray<geometry::CollisionGeometry,
                               aris::dynamic::Geometry>* pool) -> void;
  auto anchoredGeometryPool()
      -> aris::core::PointerArray<geometry::CollisionGeometry,
                                  aris::dynamic::Geometry>&;
  auto addDynamicGeometry(geometry::CollisionGeometry& dynamic_geometry)
      -> bool;
  auto addAnchoredGeometry(geometry::CollisionGeometry& anchored_geometry)
      -> bool;
  auto removeGeometry() -> bool;
  auto clearDynamicGeometry() -> bool;
  auto clearAnchoredGeometry() -> bool;
  auto updateLocation() -> bool;
  auto hasCollisions(CollisionExistsCallback& callback) -> void;
  auto init() -> void;
  CollisionEngine();
  virtual ~CollisionEngine();
  CollisionEngine(const CollisionEngine& other) = delete;
  CollisionEngine(CollisionEngine&& other) = delete;
  CollisionEngine& operator=(const CollisionEngine& other) = delete;
  CollisionEngine& operator=(CollisionEngine&& other) = delete;

 private:
  struct Imp;
  unique_ptr<Imp> imp_;
};
}  // namespace sire::collision
#endif