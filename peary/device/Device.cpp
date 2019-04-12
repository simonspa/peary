#include "config.h"
#include "device.hpp"
#include "log.hpp"

bool caribou::Device::managedDevice = false;

using namespace caribou;

Device::Device(const caribou::Configuration) {

  LOG(STATUS) << "New Caribou device instance, version " << getVersion();

  if(Device::isManaged()) {
    LOG(STATUS) << "This device is managed through the device manager.";
  } else {
    LOG(STATUS) << "Unmanaged device.";

    // Check for running device manager:
    if(check_flock("pearydevmgr.lock")) {
      throw caribou::caribouException("Found running device manager instance.");
    }

    // Acquire lock for CaribouDevice:
    if(!acquire_flock("pearydev.lock")) {
      throw caribou::caribouException("Found running device instance.");
    }
  }
}

std::string Device::getVersion() {
  return std::string(PACKAGE_STRING);
}

std::vector<std::pair<std::string, std::size_t>> Device::listCommands() {
  return _dispatcher.commands();
}

std::string Device::command(const std::string& name, const std::vector<std::string>& args) {
  try {
    return _dispatcher.call(name, args);
  } catch(std::invalid_argument& e) {
    throw caribou::ConfigInvalid(e.what());
  }
}

std::string Device::command(const std::string& name, const std::string& arg) {
  return command(name, std::vector<std::string>{arg});
}
