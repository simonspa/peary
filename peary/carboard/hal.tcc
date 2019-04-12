/**
 * Caribou HAL class implementation
 */
#ifndef CARIBOU_HAL_IMPL
#define CARIBOU_HAL_IMPL

namespace caribou {

  template <typename T>
  caribouHAL<T>::caribouHAL(std::string device_path, uint32_t device_address)
      : _devpath(device_path), _devaddress(device_address) {

    // Log the firmware
    LOG(STATUS) << getFirmwareVersion();

    T& dev_iface = InterfaceManager::getInterface<T>(_devpath);
    LOG(DEBUG) << "Prepared HAL for accessing device with interface at " << dev_iface.devicePath();

    if(!caribou::caribouHALbase::generalResetDone) { // CaR board needs to be reset
      generalReset();
    }
  }

  template <typename T> void caribouHAL<T>::writeMemory(memory_map mem, uint32_t value) { writeMemory(mem, 0, value); }

  template <typename T> void caribouHAL<T>::writeMemory(memory_map mem, size_t offset, uint32_t value) {
    iface_mem& imem = InterfaceManager::getInterface<iface_mem>(MEM_PATH);
    imem.write(mem, std::make_pair(offset, value));
  }

  template <typename T> uint32_t caribouHAL<T>::readMemory(memory_map mem) { return readMemory(mem, 0); }

  template <typename T> uint32_t caribouHAL<T>::readMemory(memory_map mem, size_t offset) {
    iface_mem& imem = InterfaceManager::getInterface<iface_mem>(MEM_PATH);
    return imem.read(mem, offset, 1).front();
  }

  template <typename T> std::string caribouHAL<T>::getFirmwareVersion() {

    const uint32_t firmwareVersion = readMemory(reg_firmware);
    const uint8_t day = (firmwareVersion >> 27) & 0x1F;
    const uint8_t month = (firmwareVersion >> 23) & 0xF;
    const uint32_t year = 2000 + ((firmwareVersion >> 17) & 0x3F);
    const uint8_t hour = (firmwareVersion >> 12) & 0x1F;
    const uint8_t minute = (firmwareVersion >> 6) & 0x3F;
    const uint8_t second = firmwareVersion & 0x3F;

    std::stringstream s;
    s << "Firmware version: " << to_hex_string(firmwareVersion) << " (" << static_cast<int>(day) << "/"
      << static_cast<int>(month) << "/" << static_cast<int>(year) << " " << static_cast<int>(hour) << ":"
      << static_cast<int>(minute) << ":" << static_cast<int>(second) << ")";

    return s.str();
  }

  template <typename T> void caribouHAL<T>::generalReset() {
    // Disable all Voltage Regulators
    LOG(DEBUG) << "Disabling all Voltage regulators";
    iface_i2c& i2c0 = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);
    i2c0.write(ADDR_IOEXP, 0x2, {0x00, 0x00}); // disable all bits of Port 1-2 (internal register)
    i2c0.write(ADDR_IOEXP, 0x6, {0x00, 0x00}); // set all bits of Port 1-2 in output mode

    LOG(DEBUG) << "Disabling all current sources";
    powerDAC(false, CUR_1.dacaddress(), CUR_1.dacoutput());
    powerDAC(false, CUR_2.dacaddress(), CUR_2.dacoutput());
    powerDAC(false, CUR_3.dacaddress(), CUR_3.dacoutput());
    powerDAC(false, CUR_4.dacaddress(), CUR_4.dacoutput());
    powerDAC(false, CUR_5.dacaddress(), CUR_5.dacoutput());
    powerDAC(false, CUR_6.dacaddress(), CUR_6.dacoutput());
    powerDAC(false, CUR_7.dacaddress(), CUR_7.dacoutput());
    powerDAC(false, CUR_8.dacaddress(), CUR_8.dacoutput());

    // In the current CaR version it has been disabled
    // Enabling DC/DC converters
    // setDCDCConverter(LTM_VPWR1, 5 );
    // setDCDCConverter(LTM_VPWR2, 5 );
    // setDCDCConverter(LTM_VPWR3, 5 );
    caribou::caribouHALbase::generalResetDone = true;
  }

  template <typename T> caribouHAL<T>::~caribouHAL() {}

  template <typename T> typename T::data_type caribouHAL<T>::send(const typename T::data_type& data) {
    return InterfaceManager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::vector<typename T::data_type> caribouHAL<T>::send(const std::vector<typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::pair<typename T::reg_type, typename T::data_type>
  caribouHAL<T>::send(const std::pair<typename T::reg_type, typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T>
  std::vector<typename T::data_type> caribouHAL<T>::send(const typename T::reg_type& reg,
                                                         const std::vector<typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_devpath).write(_devaddress, reg, data);
  }

  template <typename T>
  std::vector<std::pair<typename T::reg_type, typename T::data_type>>
  caribouHAL<T>::send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data) {
    return InterfaceManager::getInterface<T>(_devpath).write(_devaddress, data);
  }

  template <typename T> std::vector<typename T::data_type> caribouHAL<T>::receive(const unsigned int length) {
    return InterfaceManager::getInterface<T>(_devpath).read(_devaddress, length);
  }

  template <typename T>
  std::vector<typename T::data_type> caribouHAL<T>::receive(const typename T::reg_type reg, const unsigned int length) {
    return InterfaceManager::getInterface<T>(_devpath).read(_devaddress, reg, length);
  }

  template <typename T> uint32_t caribouHAL<T>::getFirmwareRegister(uint16_t) {
    throw FirmwareException("Functionality not implemented.");
  }

  template <typename T> uint8_t caribouHAL<T>::getCaRBoardID() {

    LOG(DEBUG) << "Reading board ID from CaR EEPROM";
    iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);

    // Read one word from memory address on the EEPROM:
    // FIXME register address not set!
    std::vector<uint8_t> data = myi2c.wordread(ADDR_EEPROM, 0x0, 1);

    if(data.empty())
      throw CommunicationError("No data returned");
    return data.front();
  }

  template <typename T> double caribouHAL<T>::readTemperature() {

    // Two bytes must be read, containing 12bit of temperature information plus 4bit 0.
    // Negative numbers are represented in binary twos complement format.

    LOG(DEBUG) << "Reading temperature from TMP101";
    iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);

    // Read the two temperature bytes from the TMP101:
    std::vector<uint8_t> data = myi2c.read(ADDR_TEMP, REG_TEMP_TEMP, 2);
    if(data.size() != 2)
      throw CommunicationError("No data returned");

    // FIXME correctly handle 2's complement for negative temperatures!
    int16_t temp = ((data.front() << 8) | data.back()) >> 4;
    return temp * 0.0625;
  }

  template <typename T> void caribouHAL<T>::setDCDCConverter(const DCDC_CONVERTER_T converter, const double voltage) {
    LOG(DEBUG) << "Setting " << voltage << "V "
               << "on " << converter.name();

    if(voltage > 5 || voltage < 0)
      throw ConfigInvalid("Trying to set DC/DC converter to " + std::to_string(voltage) + " V (range is 0-5 V)");

    powerDAC(false, converter.dacaddress(), converter.dacoutput());
    setDACVoltage(converter.dacaddress(), converter.dacoutput(), -0.1824 * voltage + 0.9120);
    powerDAC(true, converter.dacaddress(), converter.dacoutput());
  }

  template <typename T> void caribouHAL<T>::setBiasRegulator(const BIAS_REGULATOR_T regulator, const double voltage) {
    LOG(DEBUG) << "Setting bias " << voltage << "V "
               << "on " << regulator.name();

    if(voltage > 3.2 || voltage < 0)
      throw ConfigInvalid("Trying to set bias regulator to " + std::to_string(voltage) + " V (range is 0-3.2 V)");

    setDACVoltage(regulator.dacaddress(), regulator.dacoutput(), voltage);
  }

  template <typename T> void caribouHAL<T>::powerBiasRegulator(const BIAS_REGULATOR_T regulator, const bool enable) {

    if(enable) {
      LOG(DEBUG) << "Powering up " << regulator.name();
      powerDAC(true, regulator.dacaddress(), regulator.dacoutput());
    } else {
      LOG(DEBUG) << "Powering down " << regulator.name();
      powerDAC(false, regulator.dacaddress(), regulator.dacoutput());
    }
  }

  template <typename T>
  void caribouHAL<T>::setVoltageRegulator(const VOLTAGE_REGULATOR_T regulator,
                                          const double voltage,
                                          const double maxExpectedCurrent) {
    LOG(DEBUG) << "Setting " << voltage << "V "
               << "on " << regulator.name();

    if(voltage > 3.2 || voltage < 0)
      throw ConfigInvalid("Trying to set Voltage regulator to " + std::to_string(voltage) + " V (range is 0-3.2 V)");

    setDACVoltage(regulator.dacaddress(), regulator.dacoutput(), 3.6 - voltage);

    // set current/power monitor
    setCurrentMonitor(regulator.pwrmonitor(), maxExpectedCurrent);
  }

  template <typename T> void caribouHAL<T>::powerVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const bool enable) {

    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);

    if(enable) {
      LOG(DEBUG) << "Powering up " << regulator.name();

      // First power on DAC
      powerDAC(true, regulator.dacaddress(), regulator.dacoutput());
      // Power on the Voltage regulator
      auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
      mask |= 1 << regulator.pwrswitch();
      i2c.write(ADDR_IOEXP, std::make_pair(0x03, mask));
    } else {
      LOG(DEBUG) << "Powering down " << regulator.name();

      // Disable the Volage regulator
      auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
      mask &= ~(1 << regulator.pwrswitch());
      i2c.write(ADDR_IOEXP, std::make_pair(0x03, mask));

      // Disable the DAC
      powerDAC(false, regulator.dacaddress(), regulator.dacoutput());
    }
  }

  template <typename T>
  void caribouHAL<T>::setCurrentSource(const CURRENT_SOURCE_T source,
                                       const unsigned int current,
                                       const CURRENT_SOURCE_POLARITY_T polarity) {

    LOG(DEBUG) << "Setting " << current << "uA "
               << "on " << source.name();

    if(current > 1000)
      throw ConfigInvalid("Trying to set current source to " + std::to_string(current) + " uA (max is 1000uA)");

    // set DAC
    setDACVoltage(source.dacaddress(), source.dacoutput(), (current * CAR_VREF_4P0) / 1000);

    // set polarisation
    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);
    auto mask = i2c.read(ADDR_IOEXP, 0x02, 1)[0];

    if(polarity == CURRENT_SOURCE_POLARITY_T::PULL) {
      LOG(DEBUG) << "Polarity switch (" << to_hex_string(source.polswitch()) << ") set to PULL";
      mask &= ~(1 << source.polswitch());
    } else if(polarity == CURRENT_SOURCE_POLARITY_T::PUSH) {
      LOG(DEBUG) << "Polarity switch (" << to_hex_string(source.polswitch()) << ") set to PUSH";
      mask |= 1 << source.polswitch();
    } else {
      throw ConfigInvalid("Invalid polarity setting provided");
    }

    i2c.write(ADDR_IOEXP, std::make_pair(0x02, mask));
  }

  template <typename T> void caribouHAL<T>::powerCurrentSource(const CURRENT_SOURCE_T source, const bool enable) {
    if(enable) {
      LOG(DEBUG) << "Powering up " << source.name();
      powerDAC(true, source.dacaddress(), source.dacoutput());
    } else {
      LOG(DEBUG) << "Powering down " << source.name();
      powerDAC(false, source.dacaddress(), source.dacoutput());
    }
  }

  template <typename T>
  void caribouHAL<T>::setDACVoltage(const uint8_t device, const uint8_t address, const double voltage) {

    // Control voltages using DAC7678 with QFN packaging
    // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

    // All DAC voltage regulators on the CaR board are on the BUS_I2C3:
    LOG(DEBUG) << "Setting voltage " << voltage << "V "
               << "on DAC7678 at " << to_hex_string(device) << " channel " << to_hex_string(address);
    iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C3);

    // Per default, the internal reference is switched off,
    // with external reference we have: voltage = d_in/4096*v_refin
    uint16_t d_in = voltage * 4096 / CAR_VREF_4P0;

    // with internal reference of 2.5V we have: voltage = d_in/4096*2*2.5
    //  -> d_in = voltage/2.5 * 4096/2

    // Check out of range values
    if(d_in >= 4096)
      d_in = 4095;

    std::vector<uint8_t> command = {static_cast<uint8_t>(d_in >> 4), static_cast<uint8_t>(d_in << 4)};

    // Set DAC and update: combine command with channel via Control&Access byte:
    uint8_t reg = REG_DAC_WRUP_CHANNEL | address;

    // Send I2C write command
    myi2c.write(device, reg, command);
  }

  template <typename T> void caribouHAL<T>::powerDAC(const bool enable, const uint8_t device, const uint8_t address) {

    // Control voltages using DAC7678 with QFN packaging
    // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

    // All DAC voltage regulators on the CaR board are on the BUS_I2C3:
    LOG(DEBUG) << "Powering " << (enable ? "up" : "down") << " channel " << to_hex_string(address) << " on DAC7678 at "
               << to_hex_string(device);
    iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C3);

    // Set the correct channel bit to be powered up/down:
    uint16_t channel_bits = 2 << address;
    std::vector<uint8_t> command = {
      static_cast<uint8_t>((enable ? (REG_DAC_POWERUP | channel_bits >> 4) : (REG_DAC_POWERDOWN_HZ | channel_bits >> 4)) &
                           0xFF),
      static_cast<uint8_t>((channel_bits << 4) & 0xFF)};

    // Send I2C write command
    myi2c.write(device, REG_DAC_POWER, command);
  }

  template <typename T> void caribouHAL<T>::configureSI5345(SI5345_REG_T const* const regs, const size_t length) {
    LOG(DEBUG) << "Configuring SI5345";

    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);
    uint8_t page = regs[0].address >> 8; // first page to be used

    i2c.write(ADDR_CLKGEN, std::make_pair(0x01, page)); // set first page

    for(size_t i = 0; i < length; i++) {
      if(page != regs[i].address >> 8) { // adjust page
        page = regs[i].address >> 8;
        i2c.write(ADDR_CLKGEN, std::make_pair(0x01, page));
      }
      i2c.write(ADDR_CLKGEN, std::make_pair(regs[i].address & 0xFF, regs[i].value));
    }
  }

  template <typename T> bool caribouHAL<T>::isLockedSI5345() {
    LOG(DEBUG) << "Checking lock status of SI5345";

    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);
    i2c.write(ADDR_CLKGEN, std::make_pair(0x01, 0x00)); // set first page
    std::vector<i2c_t> rx = i2c.read(ADDR_CLKGEN, static_cast<uint8_t>(0x0E));
    if(rx[0] & 0x2) {
      LOG(DEBUG) << "SI5345 is not locked";
      return false;
    } else {
      LOG(DEBUG) << "SI5345 is locked";
      return true;
    }
  }

  template <typename T>
  void caribouHAL<T>::configurePulser(unsigned channel_mask,
                                      const bool ext_trigger,
                                      const bool ext_trig_edge,
                                      const bool idle_state) {
    // use only 4 LSBs as a mask
    channel_mask &= 0x0F;
    uint32_t pulserControl = readMemory(pulser_control);

    // select idle output state
    channel_mask <<= 8;
    if(idle_state) {
      pulserControl |= (channel_mask);
    } else {
      pulserControl &= (~channel_mask);
    }
    // set/unset ext. triggering
    channel_mask <<= 8;
    if(ext_trigger) {
      pulserControl |= (channel_mask);
    } else {
      pulserControl &= (~channel_mask);
    }
    // select ext. trigger edge
    channel_mask <<= 4;
    if(ext_trig_edge) {
      pulserControl |= (channel_mask);
    } else {
      pulserControl &= (~channel_mask);
    }

    writeMemory(pulser_control, pulserControl);

    return;
  }

  template <typename T> void caribouHAL<T>::startPulser(unsigned channel_mask) {

    // use only 4 LSBs as a mask
    channel_mask &= 0x0F;

    uint32_t pulserControl = readMemory(pulser_control);
    pulserControl |= (channel_mask);
    writeMemory(pulser_control, pulserControl);

    return;
  }

  template <typename T> void caribouHAL<T>::enablePulser(unsigned channel_mask) {

    // use only 4 LSBs as a mask
    channel_mask &= 0x0F;

    uint32_t pulserControl = readMemory(pulser_control);
    pulserControl &= ~(channel_mask << 4);
    writeMemory(pulser_control, pulserControl);

    return;
  }

  template <typename T> void caribouHAL<T>::disablePulser(unsigned channel_mask) {

    // use only 4 LSBs as a mask
    channel_mask &= 0x0F;

    uint32_t pulserControl = readMemory(pulser_control);
    pulserControl |= (channel_mask << 4);
    writeMemory(pulser_control, pulserControl);

    return;
  }

  template <typename T> uint32_t caribouHAL<T>::getPulserRunning() { return (readMemory(pulser_status) & 0x0F); }

  template <typename T> uint32_t caribouHAL<T>::getPulseCount(const uint32_t channel) {

    // check for valid channel range 1-4
    if(channel > 4 || channel < 1) {
      throw ConfigInvalid("There is no channel " + std::to_string(channel) + " available. The valid range is 1-4.");
    }

    return readMemory(pulser_counts, (CARIBOU_PULSER_CHANNEL_OFFSET * channel));
  }

  template <typename T>
  void caribouHAL<T>::configurePulseParameters(const unsigned channel_mask,
                                               const uint32_t periods,
                                               const uint32_t time_active,
                                               const uint32_t time_idle,
                                               const double voltage) {

    uint32_t channel = 1;
    uint32_t channel_int = 1;
    // for all 4 channels
    while(1) {
      // apply the settings only for channels enabled in the mask, skip others
      if(!(channel_mask & channel)) {
        continue;
      }

      switch(channel) {
      case 0b0001:
        setBiasRegulator(INJ_1, voltage);
        powerBiasRegulator(INJ_1, true);
        break;
      case 0b0010:
        setBiasRegulator(INJ_2, voltage);
        powerBiasRegulator(INJ_2, true);
        break;
      case 0b0100:
        setBiasRegulator(INJ_3, voltage);
        powerBiasRegulator(INJ_3, true);
        break;
      case 0b1000:
        setBiasRegulator(INJ_4, voltage);
        powerBiasRegulator(INJ_4, true);
        break;
      default:
        throw FirmwareException("Something went terribly wrong in caribouHAL::configurePulseParameters() and the function "
                                "reached a state where it should not be.");
      }

      writeMemory(pulser_periods, channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, periods);
      writeMemory(pulser_time_active, channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, time_active);
      writeMemory(pulser_time_idle, channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, time_idle);

      // go to next channel
      channel <<= 1;
      channel_int++;
      // check if we are still in the range of available channels. If not, we are done.
      if(!(channel & 0x0F)) {
        return;
      }
    }
  }

  template <typename T> void caribouHAL<T>::setCurrentMonitor(const uint8_t device, const double maxExpectedCurrent) {
    LOG(DEBUG) << "Setting maxExpectedCurrent " << maxExpectedCurrent << "A "
               << "on INA226 at " << to_hex_string(device);
    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C1);

    // Set configuration register:
    uint16_t conf = (1 << 14);
    // Average over 16 samples:
    conf |= (0x7 << 9);
    // Use a bus voltage conversion time of 140 us:
    conf |= (0x0 << 6);
    // Use a shunt voltage conversion time of 140us:
    conf |= (0x0 << 3);
    // Operation mode: continuous mesaurement of shunt and bus voltage:
    conf |= 0x7;
    i2c.write(device, REG_ADC_CONFIGURATION, {static_cast<i2c_t>(conf >> 8), static_cast<i2c_t>(conf & 0xFF)});

    // set calibration register

    // Calculate current LSB from expected current:
    double current_lsb = maxExpectedCurrent / (0x1 << 15);
    LOG(DEBUG) << "  current_lsb  = " << static_cast<double>(current_lsb * 1e6) << " uA/bit";

    unsigned int cal = static_cast<unsigned int>(0.00512 / (CAR_INA226_R_SHUNT * current_lsb));
    LOG(DEBUG) << "  cal_register = " << static_cast<double>(cal) << " (" << to_hex_string(cal) << ")";

    i2c.write(device, REG_ADC_CALIBRATION, {static_cast<i2c_t>(cal >> 8), static_cast<i2c_t>(cal & 0xFF)});
  }

  template <typename T> double caribouHAL<T>::measureVoltage(const VOLTAGE_REGULATOR_T regulator) {

    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C1);
    const i2c_address_t device = regulator.pwrmonitor();

    LOG(DEBUG) << "Reading bus voltage from INA226 at " << to_hex_string(device);
    std::vector<i2c_t> voltage = i2c.read(device, REG_ADC_BUS_VOLTAGE, 2);

    // INA226: fixed LSB for voltage measurement: 1.25mV
    return (static_cast<unsigned int>(voltage.at(0) << 8) | voltage.at(1)) * 0.00125;
  }

  // FIXME: somtimes it returns dummy values
  template <typename T> double caribouHAL<T>::measureCurrent(const VOLTAGE_REGULATOR_T regulator) {
    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C1);
    const i2c_address_t device = regulator.pwrmonitor();
    LOG(DEBUG) << "Reading current from INA226 at " << to_hex_string(device);

    // Reading back the calibration register:
    std::vector<i2c_t> cal_v = i2c.read(device, REG_ADC_CALIBRATION, 2);
    double current_lsb =
      static_cast<double>(0.00512) / ((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1)) * CAR_INA226_R_SHUNT);
    LOG(DEBUG) << "  current_lsb  = " << static_cast<double>(current_lsb * 1e6) << " uA/bit";

    // Reading the current register:
    std::vector<i2c_t> current_raw = i2c.read(device, REG_ADC_CURRENT, 2);
    return (static_cast<unsigned int>(current_raw.at(0) << 8) | current_raw.at(1)) * current_lsb;
  }

  // FIXME: sometimes it return dummy values
  template <typename T> double caribouHAL<T>::measurePower(const VOLTAGE_REGULATOR_T regulator) {
    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C1);
    const i2c_address_t device = regulator.pwrmonitor();
    LOG(DEBUG) << "Reading power from INA226 at " << to_hex_string(device);

    // Reading back the calibration register:
    std::vector<i2c_t> cal_v = i2c.read(device, REG_ADC_CALIBRATION, 2);
    double power_lsb =
      static_cast<double>(0.00512) / ((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1)) * CAR_INA226_R_SHUNT);
    LOG(DEBUG) << "  power_lsb  = " << static_cast<double>(power_lsb * 1e6) << " uA/bit";

    // Reading the power register:
    std::vector<i2c_t> power_raw = i2c.read(device, REG_ADC_POWER, 2);
    return (static_cast<unsigned int>(power_raw[0] << 8) | power_raw[1]) * power_lsb;
  }

  template <typename T> double caribouHAL<T>::readSlowADC(const SLOW_ADC_CHANNEL_T channel) {
    iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C3);

    LOG(DEBUG) << "Sampling channel " << channel.name() << " on pin " << static_cast<int>(channel.channel())
               << " of ADS7828 at " << to_hex_string(ADDR_ADC);

    uint8_t command = channel.address();

    // Enable single-ended mode, no differential measurement:
    command = command ^ 0x80;

    // We use the external reference voltage CAR_VREF_4P0, so do not switch to internal,
    // but keeb A/D power on
    command = command ^ 0x04;

    std::vector<i2c_t> volt_raw = i2c.read(ADDR_ADC, command, 2);
    uint16_t readback = (volt_raw[0] << 8) | volt_raw[1];
    return static_cast<double>(readback * CAR_VREF_4P0 / 4096);
  }

} // namespace caribou
#endif /* CARIBOU_HAL_IMPL */
