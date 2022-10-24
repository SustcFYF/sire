#include "sire/server/command.hpp"
#include "sire/ext/json.hpp"
#include "sire/server/interface.hpp"

namespace sire::server {
struct GetParam {
  std::vector<std::vector<double>> part_pq;  // ��ô��� body_pq?
  int state_code;
  bool is_cs_started;
  std::string currentPlan;
  int currentPlanId;

  // end_pq, end_pe, motion_pos, motion_vel, motion_acc, motion_toq, ai,
  // forcedata, forceoffset;
  /*std::vector<bool> digtal_in, digtal_out;
  std::int32_t state_code;
  aris::control::EthercatController::SlaveLinkState sls[7];
  aris::control::EthercatController::MasterLinkState mls{};
  std::vector<int> motion_state;
  std::string currentplan;
  int vel_percent;
  aris::dynamic::Marker* tool, * wobj;
  bool is_dragging, teachingmode;*/
};

auto get_state_code(aris::server::ControlServer& cs, ProgramWebInterface& inter)
    -> int {
  bool isEnable = true;
  auto& motor_pool = cs.controller().motorPool();
  for (int i = 0; i < motor_pool.size(); ++i) {
    if ((motor_pool[i].statusWord() & 0x6f) == 0x27) isEnable = false;
  }
  if (isEnable) {
    return 100;
  } else {
    if (inter.isAutoMode()) {
      if (inter.isAutoRunning()) return 400;
      if (inter.isAutoPaused()) return 410;
      if (inter.isAutoStopped()) return 420;
      return 300;
    } else {
      return 200;
    }
  }
  return 0;
}

auto Get::prepareNrt() -> void {
  option() |= NOT_RUN_EXECUTE_FUNCTION | NOT_PRINT_CMD_INFO;
  // option() |= NOT_RUN_EXECUTE_FUNCTION | NOT_RUN_COLLECT_FUNCTION;
  for (auto& m : motorOptions()) m = aris::plan::Plan::NOT_CHECK_ENABLE;
  GetParam par;
  par.part_pq.resize(model()->partPool().size());
  std::any param = par;
  auto& cs = *controlServer();
  if (cs.running()) {
    cs.getRtData(
        [&](aris::server::ControlServer& cs, const aris::plan::Plan* target,
            std::any& data) -> void {
          auto& get_param = std::any_cast<GetParam&>(data);
          auto m = dynamic_cast<aris::dynamic::Model*>(&cs.model());
          std::vector<double> temp_pq(7, 0.0);
          for (aris::Size i(-1); ++i < m->partPool().size();) {
            // par.tool->getPq(*par.wobj, std::any_cast<GetParam
            // &>(data).part_pq.data() + i * 7);
            m->partPool().at(i).getPq(temp_pq.data());
            get_param.part_pq[i].assign(temp_pq.begin(), temp_pq.end());
          }

          if (target == nullptr) {
            get_param.currentPlan = "none";
            get_param.currentPlanId = -1;
          } else {
            get_param.currentPlan = target->command().name();
            get_param.currentPlanId =
                const_cast<aris::plan::Plan*>(target)->cmdId();
          }
        },
        param);
  }
  auto out_data = std::any_cast<GetParam&>(param);

  auto& inter = dynamic_cast<ProgramWebInterface&>(cs.interfacePool().at(0));

  std::vector<std::pair<std::string, std::any>> out_param;
  out_data.state_code = get_state_code(cs, inter);
  out_data.is_cs_started = cs.running();

  // out_param.push_back(std::make_pair<std::string, std::any>("part_pq",
  // std::make_any<nlohmann::json>(std::move(js))));
  // out_param.push_back(std::make_pair<std::string, std::any>("part_pq",
  // js.dump(2)));
  out_param.push_back(std::make_pair<std::string, std::any>(
      "part_pq", nlohmann::json(out_data.part_pq)));
  out_param.push_back(
      std::make_pair<std::string, std::any>("state_code", out_data.state_code));
  out_param.push_back(std::make_pair<std::string, std::any>(
      "cs_is_started", out_data.is_cs_started));
  out_param.push_back(std::make_pair<std::string, std::any>(
      "current_plan", out_data.currentPlan));
  out_param.push_back(std::make_pair<std::string, std::any>(
      "current_plan_id", out_data.currentPlanId));
  out_param.push_back(std::make_pair(std::string("cs_err_code"),
                                     std::make_any<int>(cs.errorCode())));
  out_param.push_back(std::make_pair(
      std::string("cs_err_msg"), std::make_any<std::string>(cs.errorMsg())));
  // export const getProErrMsg = state => state.server.pro_err_msg;
  out_param.push_back(
      std::make_pair<std::string, std::any>("pro_err_msg", inter.lastError()));
  // export const getProErrCode = state => state.server.pro_err_code;
  out_param.push_back(std::make_pair<std::string, std::any>(
      "pro_err_code", inter.lastErrorCode()));
  // export const getProErrMsg = state = > state.server.pro_err_msg;
  out_param.push_back(std::make_pair<std::string, std::any>(
      "pro_err_line", inter.lastErrorLine()));
  // export const getProgramLine = state => state.server.line;
  out_param.push_back(std::make_pair<std::string, std::any>(
      "line", std::get<1>(inter.currentFileLine())));
  // export const getProgramFile = state => state.server.file;
  out_param.push_back(std::make_pair<std::string, std::any>(
      "file", std::get<0>(inter.currentFileLine())));

  ret() = out_param;
}

auto Get::collectNrt() -> void {}

Get::Get(const std::string& name) {
  aris::core::fromXmlString(
      command(),
      "<Command name=\"get\">"
      "	<GroupParam>"
      "		<Param name=\"tool\" default=\"tool0\"/>"
      "		<Param name=\"wobj\" default=\"wobj0\"/>"
      "	</GroupParam>"
      "</Command>");
}
ARIS_REGISTRATION {
  aris::core::class_<Get>("SireGet").inherit<aris::plan::Plan>();
}
}  // namespace sire::server