#include <iostream>
#include <fstream>

#include "configuration.hpp"
#include "example.hpp"
#include "log.hpp"

using namespace caribou;

int main(int argc, char* argv[]) {

  std::vector<std::string> devices;
  std::string configfile = "";
  
  // Quick and hacky cli arguments reading:
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
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

    example *dev = new example(config);

    // It's possible to call base class functions
    dev->powerOn();

    // But now also functions of the specialized class are available:
    dev->exampleCall();
    
    delete dev;
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
