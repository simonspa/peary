/**
 * Caribou Device API class implementation
 */

#ifndef CARIBOU_MIDDLEWARE_IMPL
#define CARIBOU_MIDDLEWARE_IMPL

#include "device.hpp"
#include "pearydevice.hpp"
#include "dictionary.hpp"
#include "constants.hpp"
#include "carboard.hpp"
#include "hal.hpp"
#include "log.hpp"

#include "config.h"
#include <string>

namespace caribou {

  template<typename T>
  pearyDevice<T>::pearyDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr) :
    caribouDevice(config), _hal(nullptr), _config(config) {
    LOG(logQUIET) << "New Caribou device instance, version " << getVersion();

    _hal = new caribouHAL<T>(_config.Get("devicepath",devpath),_config.Get("deviceaddress",devaddr));
  }

  template<typename T>
  pearyDevice<T>::~pearyDevice() {
    delete _hal;
  }

  template<typename T>
  std::string pearyDevice<T>::getVersion() { return std::string(PACKAGE_STRING); }

  template<typename T>
  uint8_t pearyDevice<T>::getCaRBoardID() { return _hal->getCaRBoardID(); }

  template<typename T>
  uint8_t pearyDevice<T>::getFirmwareID() { return _hal->getFirmwareRegister(ADDR_FW_ID); }

  template<typename T>
  std::string pearyDevice<T>::getDeviceName() { return std::string(); }

  template<typename T>
  void pearyDevice<T>::setVoltage(std::string name, double) {

    // Resolve name against global periphery dictionary
    LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

    // Send command to voltage regulators via HAL
    //FIXME:
    //_hal->setVoltage(_periphery.getDevice(name),_periphery.getAddress(name),voltage);
  }

  template<typename T>
  void pearyDevice<T>::voltageOn(std::string name) {

    // Resolve name against global periphery dictionary
    LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

    // Send command to voltage regulators via HAL
    //FIXME:
    //_hal->powerVoltage(true, _periphery.getDevice(name),_periphery.getAddress(name));
  }

  template<typename T>
  void pearyDevice<T>::voltageOff(std::string name) {

    // Resolve name against global periphery dictionary
    LOG(logDEBUG) << "Regulator to be configured: " << to_hex_string(_periphery.getDevice(name));

    // Send command to voltage regulators via HAL
    //FIXME:
    //  _hal->powerVoltage(false, _periphery.getDevice(name),_periphery.getAddress(name));
  }

  template<typename T>
  double pearyDevice<T>::getADC(uint8_t channel) {
    if(channel < 1 || channel > 8) {
      LOG(logCRITICAL) << "ADC channel " << std::to_string(channel) << " does not exist";
      throw caribou::ConfigInvalid("ADC channel " + std::to_string(channel) + " does not exist");
    }
  
    LOG(logDEBUG) << "Reading slow ADC, channel " << static_cast<int>(channel);
    return _hal->readSlowADC(channel);
  }

  template<typename T>
  void pearyDevice<T>::setBias(std::string, double) {}

  template<typename T>
  void pearyDevice<T>::setInjectionBias(std::string, double) {}

  template<typename T>
  void pearyDevice<T>::setCurrent(std::string, double) {}

  template<typename T>
  double pearyDevice<T>::getADC(std::string) { return 0.; }

  template<typename T>
  void pearyDevice<T>::setRegister(std::string, uint32_t value) {

    // Resolve name against registe rdictionary:
    LOG(logDEBUG) << "Register to be set: ";

    //typename T::ADDR_TYPE addr
    _hal->getInterface().send(std::make_pair(static_cast<typename T::reg_type>(0x0),static_cast<typename T::data_type>(value)));
  }


}

#endif /* CARIBOU_MIDDLEWARE_IMPL */
