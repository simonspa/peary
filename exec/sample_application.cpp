#include <fstream>
#include <iostream>

#include "DeviceManager.hpp"
#include "configuration.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;

int main(int argc, char* argv[]) {

  std::vector<std::string> devices;
  std::string configfile = "";

  // Quick and hacky cli arguments reading:
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
      std::cout << "All other arguments are interpreted as devices to be instanciated." << std::endl;
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      try {
        LogLevel log_level = Log::getLevelFromString(std::string(argv[++i]));
        Log::setReportingLevel(log_level);
      } catch(std::invalid_argument& e) {
        LOG(ERROR) << "Invalid verbosity level \"" << std::string(argv[i]) << "\", ignoring overwrite";
      }
      continue;
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else {
      std::cout << "Adding device " << argv[i] << std::endl;
      devices.push_back(std::string(argv[i]));
    }
  }

  // Create new Peary device manager
  caribou::DeviceManager* manager = new DeviceManager();

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create object:
    caribou::Configuration config;
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(WARNING) << "No configuration file provided.";
      config = caribou::Configuration();
    } else
      config = caribou::Configuration(file);

    // Demonstrate how to fetch a vector from the config file:
    std::vector<int64_t> a = config.Get("myintvec", std::vector<int64_t>());
    for(auto i : a) {
      LOG(INFO) << i;
    }

    // Spawn all devices
    for(auto d : devices) {
      // ...if we have a configuration for them
      if(config.SetSection(d)) {
        size_t device_id = manager->addDevice(d, config);
        LOG(INFO) << "Manager returned device ID " << device_id << ", fetching device...";

        // Get the device from the manager:
        Device* dev = manager->getDevice(device_id);
        dev->powerOn();

        dev->daqStart();
        dev->daqStop();
      } else {
        LOG(ERROR) << "No configuration found for device " << d;
      }
    }

    // And end that whole thing correcly:
    delete manager;
    LOG(INFO) << "Done. And thanks for all the fish.";
  } catch(caribouException& e) {
    LOG(FATAL) << "This went wrong: " << e.what();
    return -1;
  } catch(...) {
    LOG(FATAL) << "Something went terribly wrong.";
    return -1;
  }

  return 0;
}
