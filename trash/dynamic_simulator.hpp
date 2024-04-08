//
// Created by ZHOUYC on 2022/6/14.
//
#ifndef DYNAMIC_SIMULATOR_H_
#define DYNAMIC_SIMULATOR_H_

#include <sire_lib_export.h>
#include <aris.hpp>
#include <array>

namespace sire {
class SIRE_API SimulationLoop {
 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;

 private:
  SimulationLoop();
  ~SimulationLoop();
  SimulationLoop(const std::string& cs_config_path);
  SimulationLoop(const SimulationLoop&) = delete;
  SimulationLoop& operator=(const SimulationLoop&) = delete;

 public:
  static auto instance(const std::string& cs_config_path = "./sire.xml")
      -> SimulationLoop&;
  auto GetLinkPM(std::array<double, 7 * 16>& link_pm) -> void;
  auto GetLinkPQ(std::array<double, 7 * 7>& link_pq) -> void;
  auto GetLinkPE(std::array<double, 7 * 6>& link_pe) -> void;
  auto SimPlan() -> void;
  auto SimPlan(std::vector<std::array<double, 6>>) -> void;
  auto executeCmd(std::string cmd) -> void;
};
}  // namespace sire

#endif // DYNAMIC_H_
