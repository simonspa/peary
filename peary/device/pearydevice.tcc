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

namespace caribou {

  template <typename T>
  pearyDevice<T>::pearyDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr)
      : caribouDevice(config), _hal(nullptr), _config(config), _registers("Registers"), _periphery("Component"),
        _is_powered(false), _is_configured(false) {

    _hal = new caribouHAL<T>(_config.Get("devicepath", devpath), _config.Get("deviceaddress", devaddr));
  }

  template <typename T> pearyDevice<T>::~pearyDevice() { delete _hal; }

  template <typename T> std::string pearyDevice<T>::getType() { return PEARY_DEVICE_NAME; }

  template <typename T> std::string pearyDevice<T>::getFirmwareVersion() { return _hal->getFirmwareVersion(); }

  template <typename T> uint8_t pearyDevice<T>::getCaRBoardID() { return _hal->getCaRBoardID(); }

  template <typename T> uint8_t pearyDevice<T>::getFirmwareID() { return _hal->getFirmwareRegister(ADDR_FW_ID); }

  template <typename T> std::string pearyDevice<T>::getDeviceName() { return std::string(); }

  template <typename T> void pearyDevice<T>::powerOn() {
    if(_is_powered) {
      LOG(WARNING) << "Device " << getName() << " already powered.";
    } else {
      this->powerUp();
      _is_powered = true;
    }
  }

  template <typename T> void pearyDevice<T>::powerOff() {
    if(!_is_powered) {
      LOG(WARNING) << "Device " << getName() << " already off.";
    } else {
      this->powerDown();
      _is_powered = false;
      _is_configured = false;
    }
  }

  template <typename T> void pearyDevice<T>::setVoltage(std::string name, double voltage, double currentlimit) {

    // Resolve name against periphery dictionary
    std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
    LOG(DEBUG) << "Voltage to be configured: " << name << " on " << ptr->name();

    if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
      // Voltage regulators
      _hal->setVoltageRegulator(*std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr), voltage, currentlimit);
    } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
      // Bias regulators
      _hal->setBiasRegulator(*std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr), voltage);
    } else {
      throw ConfigInvalid("HAL does not provide a voltage configurator for component \"" + name + "\"");
    }
  }

  template <typename T> void pearyDevice<T>::switchPeripheryComponent(std::string name, bool enable) {

    // Resolve name against periphery dictionary
    std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
    LOG(DEBUG) << "Periphery to be switched " << (enable ? "on" : "off") << ": " << name << " on " << ptr->name();

    // Send command to appropriate switches via HAL
    if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
      // Voltage regulators
      _hal->powerVoltageRegulator(*std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr), enable);
    } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
      // Bias regulators
      _hal->powerBiasRegulator(*std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr), enable);
    } else if(std::dynamic_pointer_cast<CURRENT_SOURCE_T>(ptr)) {
      // Current sources
      _hal->powerCurrentSource(*std::dynamic_pointer_cast<CURRENT_SOURCE_T>(ptr), enable);
    } else {
      throw ConfigInvalid("HAL does not provide a switch for component \"" + name + "\"");
    }
  }

  template <typename T> double pearyDevice<T>::getVoltage(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(DEBUG) << "Voltage to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measureVoltage(*ptr);
  }

  template <typename T> double pearyDevice<T>::getCurrent(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(DEBUG) << "Current to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measureCurrent(*ptr);
  }

  template <typename T> double pearyDevice<T>::getPower(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(DEBUG) << "Power to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measurePower(*ptr);
  }

  template <typename T> double pearyDevice<T>::getADC(uint8_t channel) {
    try {
      std::vector<SLOW_ADC_CHANNEL_T> ch{VOL_IN_1, VOL_IN_2, VOL_IN_3, VOL_IN_4, VOL_IN_5, VOL_IN_6, VOL_IN_7, VOL_IN_8};
      LOG(DEBUG) << "Reading slow ADC, channel " << ch.at(channel - 1).name();
      return _hal->readSlowADC(ch.at(channel - 1));
    } catch(const std::out_of_range&) {
      LOG(FATAL) << "ADC channel " << std::to_string(channel) << " does not exist";
      throw caribou::ConfigInvalid("ADC channel " + std::to_string(channel) + " does not exist");
    }
  }

  template <typename T> double pearyDevice<T>::getADC(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<SLOW_ADC_CHANNEL_T> ptr = _periphery.get<SLOW_ADC_CHANNEL_T>(name);
    LOG(DEBUG) << "ADC channel to be sampled: " << name << " on " << ptr->name();

    // Read slow ADC
    return _hal->readSlowADC(*ptr);
  }

  template <typename T> void pearyDevice<T>::setCurrent(std::string name, int current, bool polarity) {

    // Resolve name against periphery dictionary
    std::shared_ptr<CURRENT_SOURCE_T> ptr = _periphery.get<CURRENT_SOURCE_T>(name);
    LOG(DEBUG) << "Current source to be configured: " << name << " on " << ptr->name();

    CURRENT_SOURCE_POLARITY_T pol = static_cast<CURRENT_SOURCE_POLARITY_T>(polarity);

    // Send command to current source via HAL
    _hal->setCurrentSource(*ptr, current, pol);
  }

  template <typename T> void pearyDevice<T>::setRegister(std::string name, uint32_t value) {

    // Resolve name against register dictionary:
    register_t<typename T::reg_type, typename T::data_type> reg = _registers.get(name);

    if(!reg.writable()) {
      throw caribou::RegisterTypeMismatch("Trying to write to register with \"nowrite\" flag: " + name);
    }
    if(reg.special()) {
      // Defer to special register treatment function of the derived classes:
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      setSpecialRegister(name, value);
      return;
    }

    typename T::data_type regval = static_cast<typename T::data_type>(value);
    LOG(DEBUG) << "Register to be set: " << name << " (" << to_hex_string(reg.address()) << ")";

    // Obey the mask:
    if(reg.mask() < std::numeric_limits<typename T::data_type>::max()) {
      // We need to read the register in order to preserve the nonaffected bits:
      LOG(DEBUG) << "Reg. mask:   " << to_bit_string(reg.mask());
      LOG(DEBUG) << "Shift by:    " << static_cast<int>(reg.shift());
      typename T::data_type current_reg = _hal->receive(reg.address()).front();
      LOG(DEBUG) << "new_val    = " << to_bit_string(regval);
      LOG(DEBUG) << "value (sh) = " << to_bit_string(static_cast<typename T::data_type>(regval << reg.shift()));
      LOG(DEBUG) << "curr_val   = " << to_bit_string(current_reg);
      regval = (current_reg & ~reg.mask()) | ((regval << reg.shift()) & reg.mask());
      LOG(DEBUG) << "updated    = " << to_bit_string(regval);
    } else {
      LOG(DEBUG) << "Mask covering full register: " << to_bit_string(reg.mask());
    }

    LOG(DEBUG) << "Register value to be set: " << to_hex_string(regval);
    _hal->send(std::make_pair(reg.address(), regval));

    // Cache the current value of this register:
    _register_cache[name] = value;
  }

  template <typename T> std::vector<std::pair<std::string, uint32_t>> pearyDevice<T>::getRegisters() {

    std::vector<std::pair<std::string, uint32_t>> regvalues;

    // Retrieve all registers from the device:
    std::vector<std::string> regs = _registers.getNames();
    for(auto r : regs) {
      try {
        regvalues.push_back(std::make_pair(r, this->getRegister(r)));
        LOG(DEBUG) << "Retrieved register \"" << r << "\" = " << static_cast<int>(regvalues.back().second) << " ("
                   << to_hex_string(regvalues.back().second) << ")";
      } catch(RegisterTypeMismatch& e) {
        LOG(DEBUG) << "Omitting writeonly register \"" << r << "\"";
      } catch(CommunicationError& e) {
        LOG(DEBUG) << "Failed to retrieve register \"" << r << "\" from the device.";
      }
    }

    return regvalues;
  }

  template <typename T> uint32_t pearyDevice<T>::getRegister(std::string name) {

    // Resolve name against register dictionary:
    register_t<typename T::reg_type, typename T::data_type> reg = _registers.get(name);

    if(!reg.readable()) {
      // This register cannot be read back from the device:
      throw caribou::RegisterTypeMismatch("Trying to read register with \"noread\" flag: " + name);
    }
    if(reg.special()) {
      // Defer to special register treatment function of the derived classes:
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return getSpecialRegister(name);
    }

    LOG(DEBUG) << "Register to be read: " << name << " (" << to_hex_string(reg.address()) << ")";

    typename T::data_type regval = _hal->receive(reg.address()).front();
    LOG(DEBUG) << "raw value  = " << to_bit_string(regval);
    LOG(DEBUG) << "masked val = " << to_bit_string(static_cast<typename T::data_type>(regval & reg.mask()));
    LOG(DEBUG) << "shifted val = " << static_cast<int>((regval & reg.mask()) >> reg.shift());

    // Obey the mask:
    return static_cast<uint32_t>((regval & reg.mask()) >> reg.shift());
  }

  template <typename T> void pearyDevice<T>::reset() {
    LOG(FATAL) << "Reset functionality not implemented for this device";
    throw caribou::DeviceImplException("Reset functionality not implemented for this device");
  }

  // Data return functions, for raw or decoded data
  template <typename T> std::vector<uint32_t> pearyDevice<T>::getRawData() {
    LOG(FATAL) << "Raw data readback not implemented for this device";
    throw caribou::DeviceImplException("Raw data readback not implemented for this device");
  }

  template <typename T> pearydata pearyDevice<T>::getData() {
    LOG(FATAL) << "Decoded data readback not implemented for this device";
    throw caribou::DeviceImplException("Decoded data readback not implemented for this device");
  }

  template <typename T> void pearyDevice<T>::configure() {

    if(!_is_powered) {
      LOG(ERROR) << "Device " << getName() << " is not powered!";
      return;
    }

    // Set all registers provided in the configuratio file, skip those which are not set:
    std::vector<std::string> dacs = _registers.getNames();
    LOG(INFO) << "Setting registers from configuration:";
    for(auto i : dacs) {
      try {
        uint32_t value = _config.Get<uint32_t>(i);
        this->setRegister(i, value);
        LOG(INFO) << "Set register \"" << i << "\" = " << static_cast<int>(value) << " (" << to_hex_string(value) << ")";
      } catch(ConfigMissingKey& e) {
        LOG(DEBUG) << "Could not find key \"" << i << "\" in the configuration, skipping.";
      }
    }

    _is_configured = true;
  }

  template <typename T> std::vector<std::string> pearyDevice<T>::listRegisters() { return _registers.getNames(); }

} // namespace caribou

#endif /* CARIBOU_MIDDLEWARE_IMPL */
