
//
// Created by ZHOUYC on 2022/6/14.
//
#include "sire/simulator/dynamic_simulator.hpp"
#include <algorithm>
#include <aris.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace sire {
struct Simulator::Imp {
  Simulator* simulator_;
  aris::server::ControlServer& cs_;
  std::thread retrieve_rt_pm_thead_;
  std::array<double, 7 * 16> link_pm_;
  std::mutex mu_link_pm_;

  Imp(Simulator* simulator)
      : simulator_(simulator), cs_(aris::server::ControlServer::instance()) {
    link_pm_ = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    };
  }
  Imp(const Imp&) = delete;
};

Simulator::Simulator(const std::string& cs_config_path) : imp_(new Imp(this)) {
  aris::core::fromXmlFile(imp_->cs_, cs_config_path);
  imp_->cs_.init();
  std::cout << aris::core::toXmlString(imp_->cs_) << std::endl;
  try {
    imp_->cs_.start();
    imp_->cs_.executeCmd("md");
    imp_->cs_.executeCmd("rc");
  } catch (const std::exception& err) {
    std::cout << "启动ControlServer错误，请检查配置文件" << std::endl;
    exit(1);
  }

  imp_->retrieve_rt_pm_thead_ = std::thread(
      [](aris::server::ControlServer& cs, std::array<double, 7 * 16>& link_pm,
         std::mutex& mu_link_pm) {
        const double ee[4][4]{
            {0.0, 0.0, 1.0, 0.393},
            {0.0, 1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0, 0.642},
            {0.0, 0.0, 0.0, 1.0},
        };
        while (true) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          std::any data;
          cs.getRtData(
              [&link_pm, ee](aris::server::ControlServer& cs,
                             const aris::plan::Plan* p,
                             std::any& data) -> void {
                auto m = dynamic_cast<aris::dynamic::Model*>(&cs.model());
                //获取杆件位姿
                int geoCount = 1;
                for (int i = 1; i < m->partPool().size(); ++i) {
                  auto& part = m->partPool().at(i);
                  // 获取Part位姿
                  std::array<double, 16> prtPm{1, 0, 0, 0, 0, 1, 0, 0,
                                               0, 0, 1, 0, 0, 0, 0, 1};
                  part.getPm(prtPm.data());
                  for (int j = 0; j < part.geometryPool().size(); ++j) {
                    // 获取FileGeometry位姿
                    auto geoPrtPm = dynamic_cast<aris::dynamic::FileGeometry&>(
                                        part.geometryPool().at(j))
                                        .prtPm();
                    // T_part * inv(T_fg) = 3d模型真实位姿
                    aris::dynamic::s_pm_dot_inv_pm(
                        prtPm.data(), const_cast<double*>(*geoPrtPm),
                        link_pm.data() + static_cast<long>(16) * geoCount);
                    ++geoCount;
                  }
                }
                data = link_pm;
              },
              data);
          std::lock_guard<std::mutex> guard(mu_link_pm);
          link_pm = std::any_cast<std::array<double, 16 * 7>>(data);
        }
      },
      std::ref(imp_->cs_), std::ref(imp_->link_pm_),
      std::ref(imp_->mu_link_pm_));
}

Simulator::~Simulator() {
  imp_->cs_.stop();
  imp_->cs_.close();
}

auto Simulator::GetLinkPM(std::array<double, 7 * 16>& link_pm) -> void {
  std::lock_guard<std::mutex> guard(imp_->mu_link_pm_);
  link_pm = imp_->link_pm_;
}

auto Simulator::instance(const std::string& cs_config_path) -> Simulator& {
  static Simulator instance(cs_config_path);
  return instance;
}

auto Simulator::SimPlan() -> void {
  //发送仿真轨迹
  if (imp_->retrieve_rt_pm_thead_.joinable()) {
    auto& cs = aris::server::ControlServer::instance();
    try {
      cs.executeCmd("ds");
      cs.executeCmd("md");
      cs.executeCmd("en");
      cs.executeCmd("mvj --pe={0.393, 0, 0.642, 0, 1.5708, 0}");
      cs.executeCmd("mvj --pe={0.580, 0, 0.642, 0, 1.5708, 0}");
    } catch (std::exception& e) {
      std::cout << "cs:" << e.what() << std::endl;
    }
    return;
  }
}

// ARIS_REGISTRATION { aris::core::class_<Simulator>("Simulator"); }
}  // namespace sire
