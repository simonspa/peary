#include <fstream>
#include <iostream>

#include "ExampleCaribou/ExampleCaribouDevice.hpp"
#include "utils/configuration.hpp"
#include "utils/log.hpp"

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
    }
  }

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create (const) object:
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(FATAL) << "No configuration file provided.";
      throw std::runtime_error("configuration file not found");
    }
    const caribou::Configuration config(file);

    ExampleCaribouDevice* dev = new ExampleCaribouDevice(config);

    // It's possible to call base class functions
    dev->powerOn();

    // But now also functions of the specialized class are available:
    dev->command("frobicate", "123");

    delete dev;
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
