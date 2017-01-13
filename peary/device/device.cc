/**
 * Caribou Device API class implementation
 */

#include "device.h"
#include "constants.h"
#include "hal.h"
#include "log.h"

#include <string>

using namespace caribou;

caribouDevice::caribouDevice(const caribou::Configuration config) :
  _config(config) {
  LOG(logQUIET) << "New Caribou device instance, version " << getVersion();
}

void caribouDevice::initialize() {
  LOG(logDEBUGAPI) << "Initializing Caribou device instance...";
  _hal = new caribouHAL(this->interface());
}

caribouDevice::~caribouDevice() {
  delete _hal;
}

std::string caribouDevice::getVersion() { return std::string(); }

uint8_t caribouDevice::getDeviceID() { return _hal->getDeviceID(); }

std::string caribouDevice::getDeviceName() { return std::string(); }
