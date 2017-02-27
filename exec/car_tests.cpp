#include <iostream>

#include "configuration.hpp"
#include "c3pd.hpp"
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
    std::unique_ptr<C3PD> dev( new C3PD(caribou::Configuration()) );

    dev->powerOn();
    
    while(1) {
      std::cout << "Select voltage output to configure (\"q\" to quit): ";
      std::string cmd = "";
      std::cin >> cmd;
      std::cin.sync();
      
      if(cmd == "q") break;

      std::cout << "Select voltage (in V): ";
      double v;
      std::cin >> v;
      std::cin.ignore();

      try {
	// Program voltage regulator
	dev->voltageSet(cmd,v);

	// Turn voltage on:
	dev->voltageOn(cmd);

	std::cout << "Voltage output is enabled. Press \"Enter\" to disable.";
	std::cin.ignore();

	// Turn voltage off:
	dev->voltageOff(cmd);
      }
      catch (UndefinedRegister &e) { LOG(logWARNING) << e.what(); }
    }
    
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
