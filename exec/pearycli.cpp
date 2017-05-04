#include <fstream>
#include <iostream>

#include "../extern/cpp-readline/src/Console.hpp"
#include "pearycli.hpp"

#include "configuration.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;
namespace cr = CppReadline;
using ret = cr::ReturnCode;

caribou::caribouDeviceMgr* pearycli::manager = new caribouDeviceMgr();

int main(int argc, char* argv[]) {

  std::vector<std::string> devices;
  std::string configfile = "", execfile = "";
  Log::ReportingLevel() = TLogLevel::logINFO;

  // Start peary console
  pearycli c;

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
      std::cout << "All other arguments are interpreted as devices to be instanciated." << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-r")) {
      execfile = std::string(argv[++i]);
      continue;
    } else {
      std::cout << "Adding device " << argv[i] << std::endl;
      devices.push_back(std::string(argv[i]));
    }
  }

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create object:
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(logWARNING) << "No configuration file provided, all devices will use defaults!";
      c.config = caribou::Configuration();
    } else {
      c.config = caribou::Configuration(file);
    }

    // Spawn all devices
    for(auto d : devices) {
      // ...if we have a configuration for them
      if(c.config.SetSection(d)) {
        size_t device_id = c.manager->addDevice(d, c.config);
        LOG(logINFO) << "Manager returned device ID " << device_id << ".";
      } else {
        LOG(logERROR) << "No configuration found for device " << d;
      }
    }

    // Execute provided command file if existent:
    if(!execfile.empty()) {
      c.executeFile(execfile);
    }

    LOG(logINFO) << "Welcome to pearyCLI.";
    size_t ndev = c.manager->getDevices().size();
    LOG(logINFO) << "Currently " << ndev << " devices configured.";
    if(ndev == 0) {
      LOG(logINFO) << "To add new devices use the \"add_device\" command.";
    }

    // Loop in the console until exit.
    while(c.readLine() != ret::Quit)
      ;

    LOG(logINFO) << "Done. And thanks for all the fish.";
  } catch(caribouException& e) {
    LOG(logCRITICAL) << "This went wrong: " << e.what();
    return -1;
  } catch(...) {
    LOG(logCRITICAL) << "Something went terribly wrong.";
    return -1;
  }

  return 0;
}
