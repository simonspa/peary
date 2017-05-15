/**
 * Caribou Device API class implementation
 */

#ifndef CARIBOU_MIDDLEWARE_IMPL
#define CARIBOU_MIDDLEWARE_IMPL

#include "carboard.hpp"
#include "constants.hpp"
#include "device.hpp"
#include "dictionary.hpp"
#include "hal.hpp"
#include "log.hpp"
#include "pearydevice.hpp"

#include <string>
#include "config.h"

namespace caribou {

  template <typename T>
  pearyDevice<T>::pearyDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr)
      : caribouDevice(config), _hal(nullptr), _config(config) {
    LOG(logQUIET) << "New Caribou device instance, version " << getVersion();

    _hal = new caribouHAL<T>(_config.Get("devicepath", devpath), _config.Get("deviceaddress", devaddr));
  }

  template <typename T> pearyDevice<T>::~pearyDevice() { delete _hal; }

  template <typename T> std::string pearyDevice<T>::getVersion() { return std::string(PACKAGE_STRING); }

  template <typename T> uint8_t pearyDevice<T>::getCaRBoardID() { return _hal->getCaRBoardID(); }

  template <typename T> uint8_t pearyDevice<T>::getFirmwareID() { return _hal->getFirmwareRegister(ADDR_FW_ID); }

  template <typename T> std::string pearyDevice<T>::getDeviceName() { return std::string(); }

  template <typename T> void pearyDevice<T>::setVoltage(std::string name, double voltage) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Regulator to be configured: " << name << " on " << ptr->name();

    // Send command to voltage regulators via HAL
    _hal->setVoltageRegulator(*ptr, voltage);
  }

  template <typename T> void pearyDevice<T>::voltageOn(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Regulator to be powered up: " << name << " on " << ptr->name();

    // Send command to voltage regulators via HAL
    _hal->powerVoltageRegulator(*ptr, true);
  }

  template <typename T> void pearyDevice<T>::voltageOff(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Regulator to be powered down: " << name << " on " << ptr->name();

    // Send command to voltage regulators via HAL
    _hal->powerVoltageRegulator(*ptr, false);
  }

  template <typename T> double pearyDevice<T>::getVoltage(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Voltage to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measureVoltage(*ptr);
  }

  template <typename T> double pearyDevice<T>::getCurrent(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Current to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measureCurrent(*ptr);
  }

  template <typename T> double pearyDevice<T>::getPower(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(logDEBUG) << "Power to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measurePower(*ptr);
  }

  template <typename T> double pearyDevice<T>::getADC(uint8_t channel) {
    try {
      std::vector<SLOW_ADC_CHANNEL_T> ch{VOL_IN_1, VOL_IN_2, VOL_IN_3, VOL_IN_4, VOL_IN_5, VOL_IN_6, VOL_IN_7, VOL_IN_8};
      LOG(logDEBUG) << "Reading slow ADC, channel " << ch.at(channel - 1).name();
      return _hal->readSlowADC(ch.at(channel - 1));
    } catch(const std::out_of_range&) {
      LOG(logCRITICAL) << "ADC channel " << std::to_string(channel) << " does not exist";
      throw caribou::ConfigInvalid("ADC channel " + std::to_string(channel) + " does not exist");
    }
  }

  template <typename T> double pearyDevice<T>::getADC(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<SLOW_ADC_CHANNEL_T> ptr = _periphery.get<SLOW_ADC_CHANNEL_T>(name);
    LOG(logDEBUG) << "ADC channel to be sampled: " << name << " on " << ptr->name();

    // Read slow ADC
    return _hal->readSlowADC(*ptr);
  }

  template <typename T> void pearyDevice<T>::setBias(std::string, double) {}

  template <typename T> void pearyDevice<T>::setInjectionBias(std::string, double) {}

  template <typename T> void pearyDevice<T>::setCurrent(std::string, double) {}

  template <typename T> void pearyDevice<T>::setRegister(std::string name, uint32_t value) {

    typename T::data_type regval = static_cast<typename T::data_type>(value);

    // Resolve name against register dictionary:
    register_t<typename T::reg_type, typename T::data_type> reg = _registers.get(name);
    LOG(logDEBUG) << "Register to be set: " << name << " (" << to_hex_string(reg.address()) << ")";

    // Obey the mask:
    if(reg.mask() < std::numeric_limits<typename T::data_type>::max()) {
      // We need to read the register in order to preserve the nonaffected bits:
      LOG(logDEBUG) << "Reg. mask:   " << to_bit_string(reg.mask());
      LOG(logDEBUG) << "Shift by:    " << static_cast<int>(reg.shift());
      typename T::data_type current_reg = _hal->receive(reg.address()).front();
      LOG(logDEBUG) << "new_val    = " << to_bit_string(regval);
      LOG(logDEBUG) << "value (sh) = " << to_bit_string(static_cast<typename T::data_type>(regval << reg.shift()));
      LOG(logDEBUG) << "curr_val   = " << to_bit_string(current_reg);
      regval = (current_reg & ~reg.mask()) | ((regval << reg.shift()) & reg.mask());
      LOG(logDEBUG) << "updated    = " << to_bit_string(regval);
    } else {
      LOG(logDEBUG) << "Mask covering full register: " << to_bit_string(reg.mask());
    }

    LOG(logDEBUG) << "Register value to be set: " << to_hex_string(regval);
    _hal->send(std::make_pair(reg.address(), regval));
  }

  template <typename T> uint32_t pearyDevice<T>::getRegister(std::string name) {

    // Resolve name against register dictionary:
    register_t<typename T::reg_type, typename T::data_type> reg = _registers.get(name);
    LOG(logDEBUG) << "Register to be read: " << name << " (" << to_hex_string(reg.address()) << ")";

    typename T::data_type regval = _hal->receive(reg.address()).front();
    LOG(logDEBUG) << "raw value  = " << to_bit_string(regval);
    LOG(logDEBUG) << "masked val = " << to_bit_string(static_cast<typename T::data_type>(regval & reg.mask()));
    LOG(logDEBUG) << "shifted val = " << static_cast<int>((regval & reg.mask()) >> reg.shift());

    // Obey the mask:
    return static_cast<uint32_t>((regval & reg.mask()) >> reg.shift());
  }

  // Data return functions, for raw or decoded data
  template <typename T> std::vector<uint32_t> pearyDevice<T>::getRawData() {
    LOG(logCRITICAL) << "Raw data readback not implemented for this device";
    throw caribou::NoDataAvailable("Raw data readback not implemented for this device");
  }

  template <typename T> pearydata pearyDevice<T>::getData() {
    LOG(logCRITICAL) << "Decoded data readback not implemented for this device";
    throw caribou::NoDataAvailable("Decoded data readback not implemented for this device");
  }

  template <typename T> void pearyDevice<T>::configureMatrix(std::string) {
    LOG(logCRITICAL) << "Programming of the pixel matrix not implemented for this device";
    throw caribou::NoDataAvailable("Programming of the pixel matrix not implemented for this device");
  }

  template <typename T> void pearyDevice<T>::configurePatternGenerator(std::string) {
    LOG(logCRITICAL) << "Pattern generator not implemented for this device";
    throw caribou::NoDataAvailable("Pattern generator not implemented for this device");
  }

  template <typename T> void pearyDevice<T>::triggerPatternGenerator() {
    LOG(logCRITICAL) << "Pattern generator not implemented for this device";
    throw caribou::NoDataAvailable("Pattern generator not implemented for this device");
  }

  template <typename T> void pearyDevice<T>::configure() {
    LOG(logCRITICAL) << "Device configuration not implemented for this device";
    throw caribou::NoDataAvailable("Device configuration not implemented for this device");
  }

} // namespace caribou

#endif /* CARIBOU_MIDDLEWARE_IMPL */
