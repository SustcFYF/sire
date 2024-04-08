#include <filesystem>
#include <iostream>

#include <aris.hpp>

#include "sire/middleware/sire_middleware.hpp"

auto xmlpath = std::filesystem::absolute(".");  // ��ȡ��ǰ�������ڵ�·��
const std::string xmlfile = "sire.xml";

int main(int argc, char* argv[]) {
  auto& cs = aris::server::ControlServer::instance();
  xmlpath = xmlpath / xmlfile;
  aris::core::fromXmlFile(cs, xmlpath);
  auto& simulator =
      dynamic_cast<sire::middleware::SireMiddleware&>(cs.middleWare())
          .simulationLoop();
  cs.init();
  simulator.start();

  // ��������������
  try {
    cs.start();
  } catch (const std::exception& err) {
    std::cout << "failed to start system, please reboot " << err.what() << std::endl;
  }
  // Start Web Socket
  cs.open();

  // Receive Command
  cs.runCmdLine();
  return 0;
}