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
  c.registerCommand("powerOff", powerOff);
  c.registerCommand("setVoltage", setVoltage);
  c.registerCommand("exploreInterface", exploreInterface);
  c.registerCommand("getADC", getADC);
  c.registerCommand("powerStatusLog", powerStatusLog);
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
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOn();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::powerOff(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerOff();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::setVoltage(const std::vector<std::string> & input) {
  if (input.size() < 4) {
    LOG(logINFO) << "Usage: " << input.at(0) << " OUTPUT_NAME OUTPUT_VALUE DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(3)));
    dev->setVoltage(input.at(1),std::stod(input.at(2)));
  }
  catch (caribou::caribouException &e) {
    LOG(logERROR) << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::exploreInterface(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->exploreInterface();
  }
  catch (caribou::DeviceException & e) {
    LOG(logCRITICAL) << "Exception: " << e.what();
    return ret::Error;
  }
  return ret::Ok;
}

int pearycli::getADC(const std::vector<std::string> & input) {
  if (input.size() < 3) {
    LOG(logINFO) << "Usage: " << input.at(0) << " CHANNEL_ID DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(2)));
    LOG(logINFO) << "Voltage: " << dev->getADC(std::stoi(input.at(1)));
  }
  catch (caribou::ConfigInvalid &) { return ret::Error; }
  return ret::Ok;
}

int pearycli::powerStatusLog(const std::vector<std::string> & input) {
  if (input.size() < 2) {
    LOG(logINFO) << "Usage: " << input.at(0) << " DEVICE_ID";
    return ret::Error;
  }
  try {
    caribouDevice *dev = manager->getDevice(std::stoi(input.at(1)));
    dev->powerStatusLog();
  }
  catch (caribou::DeviceException &) { return ret::Error; }
  return ret::Ok;
}
