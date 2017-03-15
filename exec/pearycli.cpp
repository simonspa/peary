#include <iostream>
#include <fstream>

#include "../extern/cpp-readline/src/Console.hpp"
#include "pearycli.hpp"

#include "configuration.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;
namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

caribou::caribouDeviceMgr * pearycli::manager = new caribouDeviceMgr();

int main(int argc, char* argv[]) {

  std::vector<std::string> devices;
  std::string configfile = "", execfile = "";
  Log::ReportingLevel() = TLogLevel::logINFO;
  
  // Start peary console
  pearycli c;
  
  // Quick and hacky cli arguments reading:
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
      std::cout << "All other arguments are interpreted as devices to be instanciated." << std::endl;
      return 0;
    }
    else if (!strcmp(argv[i],"-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
    }
    else if (!strcmp(argv[i],"-c")) {
      configfile = std::string(argv[++i]);
      continue;
    }
    else if (!strcmp(argv[i],"-r")) {
      execfile = std::string(argv[++i]);
      continue;
    }
    else {
      std::cout << "Adding device " << argv[i] << std::endl;
      devices.push_back(std::string(argv[i]));
    }
  }
  
  // Create all Caribou devices instance:
  try {

    // Open configuration file and create (const) object:
    std::ifstream file(configfile.c_str());
    if (!file.is_open()) {
      LOG(logCRITICAL) << "No configuration file provided.";
      throw std::runtime_error("configuration file not found");
    }
    const caribou::Configuration config(file);

    // Demonstrate how to fetch a vector from the config file:
    std::vector<int64_t> a = config.Get("myintvec",std::vector<int64_t>());
    for(auto i : a) { LOG(logINFO) << i; }

    // Spawn all devices
    for(auto d : devices) {
      // ...if we have a configuration for them
      if(config.SetSection(d)) {
	size_t device_id = c.manager->addDevice(d,config);
	LOG(logINFO) << "Manager returned device ID " << device_id << ", fetching device...";
      }
      else { LOG(logERROR) << "No configuration found for device " << d; }
    }

    // Execute provided command file if existent:
    if(!execfile.empty()) { c.executeFile(execfile); }
      
    // Loop in the console until exit.
    while ( c.readLine() != ret::Quit );
    
    LOG(logINFO) << "Done. And thanks for all the fish.";
  }
  catch (caribouException &e) {
    LOG(logCRITICAL) << "This went wrong: " << e.what();
    return -1;
  }
  catch (...) {
    LOG(logCRITICAL) << "Something went terribly wrong.";
    return -1;
  }
  
  return 0;
}
