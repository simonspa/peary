#include "device.hpp"
#include "config.h"
#include "log.hpp"

bool caribou::caribouDevice::managedDevice = false;

using namespace caribou;

caribouDevice::caribouDevice(const caribou::Configuration) {

  LOG(logQUIET) << "New Caribou device instance, version " << getVersion();

  if(caribouDevice::isManaged()) {
    LOG(logQUIET) << "This device is managed through the device manager.";
  } else {
    LOG(logQUIET) << "Unmanaged device.";

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
