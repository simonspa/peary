#include <fstream>
#include <iostream>
#include <signal.h>

#include "Console.hpp"
#include "pearycli.hpp"

#include "configuration.hpp"
#include "exceptions.hpp"
#include "log.hpp"

using namespace caribou;
namespace cr = CppReadline;
using ret = cr::ReturnCode;

caribou::caribouDeviceMgr* pearycli::manager = new caribouDeviceMgr();

void termination_handler(int s) {
  std::cout << "\n";
  LOG(INFO) << "Caught user signal \"" << s << "\", ending processes.";
  delete pearycli::manager;
  exit(1);
}

/**
 * @brief Clean the environment when closing application
 */
void clean() {
  Log::finish();
}

int main(int argc, char* argv[]) {
  // Add cout as the default logging stream
  Log::addStream(std::cout);

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = termination_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

  std::vector<std::string> devices;
  std::string configfile = "", execfile = "";

  // Start peary console
  pearycli c;

  // Quick and hacky cli arguments reading:
  std::string log_file_name;
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-h")) {
      std::cout << "Help:" << std::endl;
      std::cout << "-v verbosity   verbosity level, default INFO" << std::endl;
      std::cout << "-c configfile  configuration file to be used" << std::endl;
      std::cout << "-l logfile     log file to write all console output to" << std::endl;
      std::cout << "All other arguments are interpreted as devices to be instanciated." << std::endl;
      clean();
      return 0;
    } else if(!strcmp(argv[i], "-v")) {
      try {
        LogLevel log_level = Log::getLevelFromString(std::string(argv[++i]));
        Log::setReportingLevel(log_level);
      } catch(std::invalid_argument& e) {
        LOG(ERROR) << "Invalid verbosity level \"" << std::string(argv[i]) << "\", ignoring overwrite";
      }
      continue;
    } else if(strcmp(argv[i], "-l") == 0 && (i + 1 < argc)) {
      log_file_name = std::string(argv[++i]);
    } else if(!strcmp(argv[i], "-c")) {
      configfile = std::string(argv[++i]);
      continue;
    } else if(!strcmp(argv[i], "-r")) {
      execfile = std::string(argv[++i]);
      continue;
    } else {
      devices.push_back(std::string(argv[i]));
    }
  }

  // Add an extra file to log too if possible
  // NOTE: this stream should be available for the duration of the logging
  std::ofstream log_file;
  if(!log_file_name.empty()) {
    log_file.open(log_file_name, std::ios_base::out | std::ios_base::trunc);
    if(!log_file.good()) {
      LOG(FATAL) << "Cannot write to provided log file! Check if permissions are sufficient.";
      clean();
      return 1;
    }

    Log::addStream(log_file);
  }

  // Create all Caribou devices instance:
  try {

    // Open configuration file and create object:
    std::ifstream file(configfile.c_str());
    if(!file.is_open()) {
      LOG(WARNING) << "No configuration file provided, all devices will use defaults!";
      c.config = caribou::Configuration();
    } else {
      c.config = caribou::Configuration(file);
    }

    // Spawn all devices
    for(auto d : devices) {
      // ...if we have a configuration for them
      if(c.config.SetSection(d)) {
        size_t device_id = c.manager->addDevice(d, c.config);
        LOG(INFO) << "Manager returned device ID " << device_id << ".";
      } else {
        LOG(ERROR) << "No configuration found for device " << d;
      }
    }

    int retvalue = ret::Ok;

    // Execute provided command file if existent:
    if(!execfile.empty()) {
      retvalue = c.executeFile(execfile);
    }

    // Only start if everything went fine:
    if(retvalue != ret::Quit) {
      LOG(INFO) << "Welcome to pearyCLI.";
      size_t ndev = c.manager->getDevices().size();
      LOG(INFO) << "Currently " << ndev << " devices configured.";
      if(ndev == 0) {
        LOG(INFO) << "To add new devices use the \"add_device\" command.";
      }

      // Loop in the console until exit.
      while(c.readLine() != ret::Quit)
        ;
    }

    LOG(INFO) << "Done. And thanks for all the fish.";
  } catch(caribouException& e) {
    LOG(FATAL) << "This went wrong: " << e.what();
    delete c.manager;
    clean();
    return -1;
  } catch(...) {
    LOG(FATAL) << "Something went terribly wrong.";
    delete c.manager;
    clean();
    return -1;
  }

  clean();
  return 0;
}
