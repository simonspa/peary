/**
 * Caribou Device API class implementation
 */

#include "device.hpp"
#include "dictionary.hpp"
#include "constants.hpp"
#include "hal.hpp"
#include "log.hpp"

#include <string>

using namespace caribou;

dictionary<uint8_t> caribouDevice::_periphery;

caribouDevice::caribouDevice(const caribou::Configuration config) :
  _hal(nullptr), _config(config) {
  LOG(logQUIET) << "New Caribou device instance, version " << getVersion();
}

void caribouDevice::initialize(std::string devpath, caribou::dictionary<uint8_t> periphery) {
  LOG(logDEBUGAPI) << "Initializing Caribou device instance...";
  _hal = new caribouHAL(this->interface(),_config.Get("devicepath",devpath));

  // Supplement the periphery dictionary with local names and definitions:
  _periphery += periphery;
}

caribouDevice::~caribouDevice() {
  delete _hal;
}

std::string caribouDevice::getVersion() { return std::string(); }

uint8_t caribouDevice::getCaRBoardID() { return _hal->getCaRBoardID(); }

uint8_t caribouDevice::getFirmwareID() { return _hal->getFirmwareRegister(ADDR_FW_ID); }

std::string caribouDevice::getDeviceName() { return std::string(); }

void caribouDevice::setVoltage(std::string name, double voltage) {

  // Resolve name against global periphery dictionary
  LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

  // Send command to voltage regulators via HAL
  _hal->setVoltage(_periphery.getDevice(name),_periphery.getAddress(name),voltage);
}
