/**
 * Caribou Device API class implementation
 */

#include "device.h"
#include "hal.h"

#include <string>

using namespace caribou;

caribouDevice::caribouDevice() {
  _hal = new caribouHAL();
}

caribouDevice::~caribouDevice() {
  delete _hal;
}

std::string caribouDevice::getVersion() { return std::string(); }

uint8_t caribouDevice::getDeviceID() { return _hal->getDeviceID(); }

std::string caribouDevice::getDeviceName() { return std::string(); }
