#include "device.hpp"
#include "config.h"
#include "log.hpp"

bool caribou::caribouDevice::managedDevice = false;

using namespace caribou;

caribouDevice::caribouDevice(const caribou::Configuration) {

  LOG(STATUS) << "New Caribou device instance, version " << getVersion();

  if(caribouDevice::isManaged()) {
    LOG(STATUS) << "This device is managed through the device manager.";
  } else {
    LOG(STATUS) << "Unmanaged device.";

    // Check for running device manager:
    if(check_flock("pearydevmgr.lock")) {
      throw caribou::caribouException("Found running device manager instance.");
    }

    // Acquire lock for pearydevice:
    if(!acquire_flock("pearydev.lock")) {
      throw caribou::caribouException("Found running device instance.");
    }
  }
}

std::string caribouDevice::getVersion() {
  return std::string(PACKAGE_STRING);
}

std::vector<std::pair<std::string, std::size_t>> caribouDevice::list_commands() {
  return _dispatcher.commands();
}

void caribouDevice::command(const std::string& name, const std::vector<std::string>& args) {
  try {
    std::cout << _dispatcher.call(name, args) << '\n';
  } catch(std::invalid_argument& e) {
    throw caribou::ConfigInvalid(e.what());
  }
}

void caribouDevice::command(const std::string& name, const std::string& arg) {
  command(name, std::vector<std::string>{arg});
}
