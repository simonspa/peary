#include <iostream>

#include "configuration.hpp"
#include "example.hpp"
#include "log.hpp"

using namespace caribou;

int main(int argc, char* argv[]) {

  // Quick and hacky cli arguments reading:
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      return 0;
    }
    else if (!strcmp(argv[i],"-v")) {
      Log::ReportingLevel() = Log::FromString(std::string(argv[++i]));
      continue;
    }
  }
  
  // Create all Caribou devices instance:
  try {
    example *dev = new example(caribou::Configuration());

    while(1) {
      std::string cmd = "";
      std::cout << "Select voltage output to configure (\"exit\" to quit): ";
      std::cin >> cmd;
      if(cmd == "exit") break;

      double v;
      std::cout << "Select voltage (in V): ";
      std::cin >> v;

      try {
	// Program voltage regulator
	dev->voltageSet(cmd,v);
      }
      catch (UndefinedRegister &e) { LOG(logWARNING) << e.what(); }
    }
    
    delete dev;
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
