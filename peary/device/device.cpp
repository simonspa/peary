/**
 * Caribou Device API class implementation
 */

#include "device.hpp"
#include "dictionary.hpp"
#include "constants.hpp"
#include "carboard.hpp"
#include "hal.hpp"
#include "log.hpp"

#include "config.h"
#include <string>

using namespace caribou;

caribouDevice::caribouDevice(const caribou::Configuration config) :
  _hal(nullptr), _config(config) {
  LOG(logQUIET) << "New Caribou device instance, version " << getVersion();
}

void caribouDevice::initialize(std::string devpath, uint32_t devaddr, caribou::dictionary<uint8_t> periphery) {
  LOG(logDEBUGAPI) << "Initializing Caribou device instance...";
  _hal = new caribouHAL(this->interface(),_config.Get("devicepath",devpath),_config.Get("deviceaddress",devaddr));

  // Supplement the periphery dictionary with local names and definitions:
  _periphery += periphery;
}

caribouDevice::~caribouDevice() {
  delete _hal;
}

std::string caribouDevice::getVersion() { return std::string(PACKAGE_STRING); }

uint8_t caribouDevice::getCaRBoardID() { return _hal->getCaRBoardID(); }

uint8_t caribouDevice::getFirmwareID() { return _hal->getFirmwareRegister(ADDR_FW_ID); }

std::string caribouDevice::getDeviceName() { return std::string(); }

void caribouDevice::setVoltage(std::string name, double voltage) {

  // Resolve name against global periphery dictionary
  LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

  // Send command to voltage regulators via HAL
  //FIXME:
  //_hal->setVoltage(_periphery.getDevice(name),_periphery.getAddress(name),voltage);
}

void caribouDevice::voltageOn(std::string name) {

  // Resolve name against global periphery dictionary
  LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

  // Send command to voltage regulators via HAL
  //FIXME:
  //_hal->powerVoltage(true, _periphery.getDevice(name),_periphery.getAddress(name));
}

void caribouDevice::voltageOff(std::string name) {

  // Resolve name against global periphery dictionary
  LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

  // Send command to voltage regulators via HAL
  //FIXME:
  //  _hal->powerVoltage(false, _periphery.getDevice(name),_periphery.getAddress(name));
}

double caribouDevice::getADC(uint8_t channel) {
  if(channel < 1 || channel > 8) {
    LOG(logCRITICAL) << "ADC channel " << std::to_string(channel) << " does not exist";
    throw caribou::ConfigInvalid("ADC channel " + std::to_string(channel) + " does not exist");
  }
  
  LOG(logDEBUG) << "Reading slow ADC, channel " << static_cast<int>(channel);
  return _hal->readSlowADC(channel);
}

