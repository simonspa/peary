#include "../extern/cpp-readline/src/Console.hpp"
#include "log.hpp"
#include "pearycli.hpp"

using namespace caribou;
using ret = CppReadline::Console::ReturnCode;

pearycli::pearycli() : c("# ") {

  // Register console commands
  c.registerCommand("list_devices", devices);
  c.registerCommand("verbosity", verbosity);
  c.registerCommand("powerOn", powerOn);
}

pearycli::~pearycli() {
  // Delete the device manager
  delete manager;
}

int pearycli::devices(const std::vector<std::string> &) {

  try {
    size_t i = 0;
    std::vector<caribouDevice*> devs = manager->getDevices();
    for(auto d : devs) {
      LOG(logINFO) << "ID " << i << ": " << d->getName();
      i++;
    }
  }
  catch (caribou::DeviceException &e) {}
  return ret::Ok;
}

int pearycli::verbosity(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " LOGLEVEL";
    return ret::Error;
  }
  Log::ReportingLevel() = Log::FromString(input.at(1));
  return ret::Ok;
}


int pearycli::powerOn(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
  dev->powerOn();
  return ret::Ok;
}
