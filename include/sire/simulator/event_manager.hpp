#ifndef SIRE_EVENT_MANAGER_HPP_
#define SIRE_EVENT_MANAGER_HPP_
#include <any>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sire_lib_export.h>

#include <aris/core/object.hpp>
#include <aris/server/control_server.hpp>

#include "sire/core/sorted_pair.hpp"
#include "sire/physics/common/penetration_as_point_pair.hpp"
#include "sire/physics/physics_engine.hpp"

namespace sire::simulator {
class Simulator;
enum class EventFeature {
  START,          // ��ʼ
  END,            // ����
  CONTACT_START,  // �Ӵ���ʼ
  CONTACT_END,    // ����Ӵ�
  NORMAL_STEP,    // ����ʱ�䲽��
};

using CollisionResultMap =
    std::unordered_map<core::SortedPair<double>,
                       std::vector<physics::common::PenetrationAsPointPair>>;
// TODO(leitianjian): Ӧ����һ��������ʵ��Event�Ĳ�������һЩ
struct Event {
  std::unordered_set<EventFeature> functionalities_;
  CollisionResultMap contact_start_pair_;
  std::unordered_set<core::SortedPair<double>> contact_end_pair_;
  double time_;
};
/// <summary>
///  EventManager����������������������������е����������PhysicalEngine��Model�н����
///  ������ʵ������EventManager������
/// </summary>
class EventManager {
 public:
  auto init() -> void;

  /**
  * ��������һ���¼�ʱ��
  * @returns ����ɹ�������`true`����ǰʱ�䲽�����¼�ʱ�䣬
             ���ʧ�ܣ�����`false`����ǰʱ�䲻����
  */
  auto step(double dt) -> bool;
  auto start() -> void;

  auto getModelState(const std::function<void(aris::server::ControlServer&,
                                              Simulator&, std::any&)>& get_func,
                     std::any& get_data) -> void;

  EventManager();
  virtual ~EventManager();
  SIRE_DECLARE_MOVE_CTOR(EventManager);

 private:
  auto resolveEvent(Event& e) -> void;
  auto preprocessEvent(Event& e) -> bool;

  auto resolveStartEvent(Event& e) -> void;
  auto resolveContactStartEvent(Event& e) -> void;
  auto resolveContactEndEvent(Event& e) -> void;
  auto resolveNormalStepEvent(Event& e) -> void;

  auto preprocessContactStartEvent(Event& e) -> void;
  auto preprocessContactEndEvent(Event& e) -> void;
  auto preprocessNormalStepEvent(Event& e) -> void;
  auto preprocessEndEvent(Event& e) -> void;

  auto insertEventAt(Event&& e, std::list<Event>::const_iterator it,
                     bool reverse = false) -> void;
  auto addNextStepEventAt(double time, std::list<Event>::const_iterator it,
                          bool reverse = false) -> void;
  auto addPrevContactStartEventAt(double time,
                                  CollisionResultMap&& collision_result,
                                  std::list<Event>::const_iterator it,
                                  bool reverse = false) -> void;

  // init ptr from parent object Simulator
  physics::PhysicsEngine* engine_ptr_;
  Simulator* simulator_ptr_;
  // config from simulator
  double dt_;
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
  friend class Simulator;
};
}  // namespace sire::simulator
#endif