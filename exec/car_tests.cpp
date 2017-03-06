#include <iostream>

#include "configuration.hpp"
#include "clicpix2.hpp"
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
    std::unique_ptr<C3PD> c3pd( new C3PD(caribou::Configuration()) );
    std::unique_ptr<clicpix2> cpx2( new clicpix2(caribou::Configuration()) );

    cpx2->init();
    c3pd->init();
    
    while(1) {
      std::cout << "Press \"q\" to quit or \"m\" to measure: ";
      std::string cmd = "";
      std::cin >> cmd;
      std::cin.sync();
      
      if(cmd == "q") break;
      if(cmd == "m"){
	cpx2->powerStatusLog();
	c3pd->powerStatusLog();
      }
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
