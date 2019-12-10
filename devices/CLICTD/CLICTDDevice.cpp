
/**
 * Caribou implementation for the CLICTD
 */

#include "CLICTDDevice.hpp"
#include "utils/log.hpp"

#include <fstream>

using namespace caribou;

CLICTDDevice::CLICTDDevice(const caribou::Configuration config)
    : CaribouDevice(config, std::string(DEFAULT_DEVICEPATH), CLICTD_DEFAULT_I2C) {

  _dispatcher.add("powerStatusLog", &CLICTDDevice::powerStatusLog, this);
  _dispatcher.add("configureMatrix", &CLICTDDevice::configureMatrix, this);
  _dispatcher.add("setOutputMultiplexer", &CLICTDDevice::setOutputMultiplexer, this);
  _dispatcher.add("configureClock", &CLICTDDevice::configureClock, this);
  _dispatcher.add("getMemory", &CLICTDDevice::getMem, this);
  _dispatcher.add("setMemory", &CLICTDDevice::setMem, this);
  _dispatcher.add("triggerPatternGenerator", &CLICTDDevice::triggerPatternGenerator, this);
  _dispatcher.add("configurePatternGenerator", &CLICTDDevice::configurePatternGenerator, this);
  _dispatcher.add("clearTimestamps", &CLICTDDevice::clearTimestamps, this);

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);
  _periphery.add("vdda", PWR_OUT_6);
  _periphery.add("pwell", PWR_OUT_8);
  _periphery.add("sub", PWR_OUT_3);

  _periphery.add("analog_in", BIAS_1);
  _periphery.add("ref", BIAS_2);

  _periphery.add("analog_out", VOL_IN_1);
  _periphery.add("dac_out", VOL_IN_1);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(CLICTD_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(CLICTD_MEMORY);

  // Matrix not configured yet:
  matrixConfigured = false;
}

void CLICTDDevice::getMem(std::string name) {
  auto value = getMemory(name);
  LOG(INFO) << name << " = 0x" << to_hex_string(value);
}

void CLICTDDevice::setMem(std::string name, uint32_t value) {
  setMemory(name, value);
}

void CLICTDDevice::configure() {
  LOG(INFO) << "Configuring";
  configureClock(_config.Get<bool>("clock_internal", true));
  reset();
  mDelay(10);

  // Call the base class configuration function:
  CaribouDevice<iface_i2c>::configure();

  // Read pattern generator from the configuration and program it:
  std::string pg = _config.Get("patterngenerator", "");
  if(!pg.empty()) {
    LOG(INFO) << "Found pattern generator in configuration, programming file \"" << pg << "\"...";
    configurePatternGenerator(pg);
  } else {
    LOG(INFO) << "No pattern generator found in configuration.";
  }

  if(!matrixConfigured) {
    // Read matrix file from the configuration and program it:
    std::string matrix = _config.Get("matrix", "");
    if(!matrix.empty()) {
      LOG(INFO) << "Found pixel matrix setup in configuration, programming file \"" << matrix << "\"...";
      configureMatrix(matrix);
    } else {
      LOG(INFO) << "No pixel matrix configuration setting found.";
    }
  } else {
    LOG(INFO) << "Matrix was already configured. Skipping.";
  }

  // CLICTD signal order (from LSB):
  //            T0, Reset, Shutter, TP, Pwr, RO_start, RO_active
  //            1   2      4        8   16   32        64
  // Stored in topmost 16 bits of 64bit timestamp
  // Configure, which timestamps we would like to see:
  std::string ts_trigger_string = _config.Get("timestamp_triggers", "");
  std::stringstream ss(ts_trigger_string);
  size_t pos = 0;
  uint32_t ts_triggers = 0;
  while(ss.good()) {
    std::string substr;
    getline(ss, substr, ',');
    LOG(DEBUG) << "TS_TRG: Found timestamp trigger " << substr << ", pos " << (pos * 2);
    uint32_t value = 0;
    if(substr == "RISING" || substr == "R") {
      value = (0b01 << (pos * 2));
    } else if(substr == "FALLING" || substr == "F") {
      value = (0b10 << (pos * 2));
    } else if(substr == "EDGE" || substr == "E") {
      value = (0b11 << (pos * 2));
    } else if(substr == "NONE" || substr == "N") {
    } else {
      LOG(ERROR) << "Unknown timestamp trigger " << substr << " - disabling this signal (0x00)";
    }
    LOG(DEBUG) << "TS_TRG: Adding value " << to_bit_string(value, 32, true);
    ts_triggers |= value;
    pos++;
  }
  LOG(INFO) << "Setting timestamp triggers to " << to_bit_string(ts_triggers, 32, true) << "(" << ts_triggers << ")";
  setMemory("tsedgeconf", ts_triggers);

  // Enable recording of timestamps:
  setMemory("tscontrol", 0x3);
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void CLICTDDevice::configurePatternGenerator(std::string filename) {

  LOG(DEBUG) << "Resetting pattern generator configuration";
  setMemory("wgcontrol", 0x4);

  LOG(DEBUG) << "Programming pattern generator";
  std::vector<uint32_t> patterns;

  std::ifstream pgfile(filename);
  if(!pgfile.is_open()) {
    LOG(ERROR) << "Could not open pattern generator configuration file \"" << filename << "\"";
    throw ConfigInvalid("Could not open pattern generator configuration file \"" + filename + "\"");
  }

  enum class TriggerConditionGlobal : uint8_t {
    OR = 0b001,
    NOR = 0b101,
    AND = 0b011,
    NAND = 0b111,
    XOR = 0b010,
    XNOR = 0b110,
    TRUE = 0b000
  };

  enum class TriggerConditionLocal : uint8_t {
    HIGH = 0b100,
    LOW = 0b101,
    RISING = 0b001,
    FALLING = 0b010,
    EDGE = 0b011,
    ALWAYS = 0b111,
    NEVER = 0b000
  };

  std::string line = "";
  while(std::getline(pgfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pgline(line);
    std::string triggerconditions;
    std::string condition;
    std::string signals;
    uint32_t duration;
    if(pgline >> triggerconditions >> condition >> signals >> duration) {

      uint8_t output = 0;
      std::stringstream ss(signals);
      while(ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        if(substr == "RO") {
          output |= CLICTD_READOUT_START;
        } else if(substr == "PWR") {
          output |= CLICTD_POWER_ENABLE;
        } else if(substr == "TP") {
          output |= CLICTD_TESTPULSE;
        } else if(substr == "SH") {
          output |= CLICTD_SHUTTER;
        } else if(substr == "RE") {
          output |= CLICTD_RESET;
        } else if(substr == "PLS") {
          output |= CLICTD_PULSER;
        } else if(substr == "NONE") {
        } else {
          LOG(ERROR) << "Unrecognized pattern for pattern generator: " << substr << " - ignoring.";
        }
      }

      uint16_t triggers = 0;
      if(condition == "OR") {
        triggers = as_value(TriggerConditionGlobal::OR);
      } else if(condition == "NOR") {
        triggers = as_value(TriggerConditionGlobal::NOR);
      } else if(condition == "AND") {
        triggers = as_value(TriggerConditionGlobal::AND);
      } else if(condition == "NAND") {
        triggers = as_value(TriggerConditionGlobal::NAND);
      } else if(condition == "XOR") {
        triggers = as_value(TriggerConditionGlobal::XOR);
      } else if(condition == "XNOR") {
        triggers = as_value(TriggerConditionGlobal::XNOR);
      } else if(condition == "TRUE") {
        triggers = as_value(TriggerConditionGlobal::TRUE);
      } else {
        LOG(ERROR) << "Unrecognized global trigger condition for pattern generator: " << condition;
        throw ConfigInvalid("Invalid global trigger condition");
      }

      size_t n_triggers = 0;
      std::stringstream ss2(triggerconditions);
      while(ss2.good()) {
        std::string substr;
        getline(ss2, substr, ',');
        if(substr == "HIGH" || substr == "H") {
          triggers |= (as_value(TriggerConditionLocal::HIGH) << (n_triggers + 1) * 3);
        } else if(substr == "LOW" || substr == "L") {
          triggers |= (as_value(TriggerConditionLocal::LOW) << (n_triggers + 1) * 3);
        } else if(substr == "RISING" || substr == "R") {
          triggers |= (as_value(TriggerConditionLocal::RISING) << (n_triggers + 1) * 3);
        } else if(substr == "FALLING" || substr == "F") {
          triggers |= (as_value(TriggerConditionLocal::FALLING) << (n_triggers + 1) * 3);
        } else if(substr == "EDGE" || substr == "E") {
          triggers |= (as_value(TriggerConditionLocal::EDGE) << (n_triggers + 1) * 3);
        } else if(substr == "ALWAYS" || substr == "A") {
          triggers |= (as_value(TriggerConditionLocal::ALWAYS) << (n_triggers + 1) * 3);
        } else if(substr == "NEVER" || substr == "N") {
          triggers |= (as_value(TriggerConditionLocal::NEVER) << (n_triggers + 1) * 3);
        } else {
          LOG(ERROR) << "Unrecognized signal trigger condition for pattern generator: " << substr << " - setting to NEVER.";
          triggers |= (as_value(TriggerConditionLocal::NEVER) << (n_triggers + 1) * 3);
        }
        n_triggers++;
      }

      LOG(DEBUG) << "PG: setting duration " << duration << " clk";
      setMemory("wgpatterntime", duration);
      LOG(DEBUG) << "PG: setting output " << to_bit_string(output, 8, true);
      setMemory("wgpatternoutput", output);
      LOG(DEBUG) << "PG: setting trigger conditions " << to_bit_string(triggers, (n_triggers + 1) * 3, true);
      setMemory("wgpatterntriggers", triggers);

      // Trigger the write:
      setMemory("wgcontrol", 0x8);

      auto remaining = getMemory("wgcapacity");
      LOG(INFO) << "Added slot with " << n_triggers << " trigger signals to pattern generator, " << remaining
                << " slots remaining";
    }
  }

  LOG(DEBUG) << "Setting to run PG once";
  setMemory("wgconfruns", 1);

  LOG(INFO) << "Done configuring pattern generator.";
}

void CLICTDDevice::reset() {
  LOG(DEBUG) << "Resetting";

  // assert reset:
  setMemory("chipcontrol", 0b100000);
  usleep(5);
  // deny reset:
  setMemory("chipcontrol", 0);
}

CLICTDDevice::~CLICTDDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void CLICTDDevice::configureClock(bool internal) {

  LOG(DEBUG) << "Configuring Si5345 clock source";
  _hal->configureSI5345((SI5345_REG_T const* const)si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
  mDelay(100); // let the PLL lock

  // If required, check whether we are locked to external clock:
  if(!internal) {
    LOG(DEBUG) << "Waiting for clock to lock...";
    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock.");
    }
  }
}

std::vector<uint32_t> CLICTDDevice::getRawData() {
  triggerPatternGenerator(true);

  LOG(DEBUG) << "Preparing raw data packet";
  std::vector<uint32_t> rawdata;

  // Get the timestamps:
  auto timestamps = getTimestamps();

  // Read frame data:
  auto frame = getFrame();

  // Pack data: first data block is number of timestamp words, followed by timestamps and frame data
  rawdata.push_back(timestamps.size());
  rawdata.insert(rawdata.end(), timestamps.begin(), timestamps.end());
  rawdata.insert(rawdata.end(), frame.begin(), frame.end());

  LOG(DEBUG) << "Raw data packet with " << rawdata.size() << " words ready";
  return rawdata;
}

std::vector<uint32_t> CLICTDDevice::getFrame(bool manual_readout) {

  if(manual_readout) {
    // Manually trigger readout:
    LOG(DEBUG) << "Frame readout requested";
    setMemory("rdcontrol", 1);
    uint32_t attempts = 0;
    while((getMemory("rdstatus")) & 0x20) {
      usleep(100);
      if(attempts++ >= 16384) {
        LOG(ERROR) << "Frame readout timeout";
        return std::vector<uint32_t>();
      }
    }
  }

  // Poll data until there nothing something left anymore
  std::vector<uint32_t> rawdata;
  while((getMemory("rdstatus")) & 0b1) {
    LOG(TRACE) << "Reading word " << rawdata.size() << " from FIFO";
    uint32_t data = getMemory("rdfifo");
    rawdata.push_back(data);
  }
  LOG(DEBUG) << "Read " << rawdata.size() << " 32bit words from FIFO.";
  return rawdata;
}

void CLICTDDevice::clearTimestamps() {
  // Retrieve all timestamps and throw them away:
  getTimestamps();
}

std::vector<uint32_t> CLICTDDevice::getTimestamps() {

  LOG(DEBUG) << "Requesting timestamps";

  if((getMemory("tsstatus") & 0x1) == 0) {
    LOG(WARNING) << "Timestamps FIFO is empty";
    return std::vector<uint32_t>();
  }

  std::vector<uint32_t> timestamps;
  do {
    // Read LSB and MSB of timestamp
    uint32_t ts_lsb = getMemory("tsfifodata_lsb");
    uint32_t ts_msb = getMemory("tsfifodata_msb");

    timestamps.push_back(ts_msb);
    timestamps.push_back(ts_lsb);
    LOG(DEBUG) << ts_msb << " | " << ts_lsb << "\t= " << ((static_cast<uint64_t>(ts_msb) << 32) | ts_lsb);
  } while(getMemory("tsstatus") & 0x1);

  LOG(DEBUG) << "Received " << timestamps.size() / 2 << " timestamps: " << listVector(timestamps, ",", false);

  return timestamps;
}

CLICTDDevice::matrixConfig CLICTDDevice::readMatrix(std::string filename) const {

  matrixConfig pixelsConfig;
  size_t masked = 0;
  LOG(DEBUG) << "Reading pixel matrix file.";
  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    throw ConfigInvalid("Could not open matrix file \"" + filename + "\"");
  }

  std::string line = "";
  while(std::getline(pxfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pxline(line);
    int column, row, mask, tp_dig, tp_analog;
    int threshold0, threshold1, threshold2, threshold3, threshold4, threshold5, threshold6, threshold7;
    if(pxline >> column >> row >> mask >> tp_dig >> tp_analog >> threshold0 >> threshold1 >> threshold2 >> threshold3 >>
       threshold4 >> threshold5 >> threshold6 >> threshold7) {

      // Prepare thresholds:
      std::vector<uint8_t> thresholds;
      thresholds.push_back(threshold0);
      thresholds.push_back(threshold1);
      thresholds.push_back(threshold2);
      thresholds.push_back(threshold3);
      thresholds.push_back(threshold4);
      thresholds.push_back(threshold5);
      thresholds.push_back(threshold6);
      thresholds.push_back(threshold7);

      pixelsConfig[std::make_pair(column, row)] =
        std::make_pair(pixelConfigStage1(static_cast<uint8_t>(mask), tp_dig, static_cast<uint8_t>(tp_analog), thresholds),
                       pixelConfigStage2(thresholds));
      if(mask > 0)
        masked++;
    }
  }
  LOG(INFO) << pixelsConfig.size() << " superpixel configurations cached, " << masked
            << " of which are at least partly masked";
  return pixelsConfig;
}

void CLICTDDevice::configureMatrix(std::string filename) {

  if(!filename.empty()) {
    LOG(DEBUG) << "Configuring the pixel matrix from file \"" << filename << "\"";
    pixelConfiguration = readMatrix(filename);
  }

  // Retry programming matrix:
  int retry = 0;
  int retry_max = _config.Get<int>("retry_matrix_config", 3);
  while(true) {
    try {
      programMatrix();
      break;
    } catch(caribou::DataException& e) {
      LOG(ERROR) << e.what();
      if(++retry == retry_max) {
        throw CommunicationError("Matrix configuration failed");
      }
      LOG(INFO) << "Repeating configuration attempt";
    }
  }
  matrixConfigured = true;
}

void CLICTDDevice::programMatrix() {

  // Get status of compression:
  auto compression = getRegister("nocompress");
  // Switch compression off:
  setRegister("nocompress", true);

  auto check_clk_stopped = [this]() {
    // check if readout clock is running
    int retry = 0;
    while(!(getMemory("rdstatus") & 0x10)) {
      if(++retry > 3) {
        LOG(ERROR) << "Readout clock still running despite being in the matrix configuration mode.";
        return false;
      }
    }
    return true;
  };

  auto check_clk_running = [this]() {
    // check if readout clock is stopped
    int retry = 0;
    while(getMemory("rdstatus") & 0x10) {
      if(++retry > 3) {
        LOG(ERROR) << "Readout clock not running despite the matrix configuration mode should be off.";
        return false;
      }
    }
    return true;
  };

  auto configure_stage = [this](bool first_stage) {
    // Follow procedure described in chip manual, section 4.1 to configure the matrix:
    auto bitvalues = [](matrixConfig config, bool stage1, size_t row, size_t bit) {
      uint16_t bits = 0;
      for(uint8_t column = 0; column < 16; column++) {
        bool value;
        if(stage1) {
          value = config[std::make_pair(column, row)].first.GetBit(bit);
        } else {
          value = config[std::make_pair(column, row)].second.GetBit(bit);
        }
        bits |= (value << column);
      }
      return bits;
    };

    // For each of the pixels per column, do
    for(size_t row = 0; row < 128; row++) {
      // Read configuration bits for STAGE 1 one by one:
      for(size_t bit = 22; bit > 0; bit--) {
        // Load ’configData’ register with bit 21 of the 1st configuration stage (1 bit per column)
        auto value = bitvalues(pixelConfiguration, first_stage, row, bit - 1);
        LOG(DEBUG) << "row: " << row << ", bit: " << bit << ", data: " << value;
        int retry = 0;
        while(retry <= CLICTD_MAX_CONF_RETRY) {
          // Write the value to ’configData’ register
          this->setRegister("configdata", value);
          // Write 0x11/0x12 to ’configCtrl’ register to shift configuration in the matrix
          this->setRegister("configctrl", 0x10 | (first_stage ? 0x01 : 0x02));
          // Write 0x01/0x02 to ’configCtrl’ register
          this->setRegister("configctrl", 0x00 | (first_stage ? 0x01 : 0x02));
          // Repeat until the clock pulse was generated
          if((getMemory("rdstatus") & 0x6) == 0x6) {
            break;
          }
          retry++;
        }
        if(retry > 0) {
          if(retry > CLICTD_MAX_CONF_RETRY) {
            // Too many attempts was made, We gave up.
            LOG(ERROR) << "Could not generate matrix write clock pulse at configuration stage "
                       << std::to_string(first_stage ? 1 : 2) << " for row " << row << ", bit " << bit << " after " << retry
                       << " attempts.";
          } else {
            LOG(DEBUG) << "Needed to repeat matrix write clock pulse at configuration stage "
                       << std::to_string(first_stage ? 1 : 2) << " for row " << row << ", bit " << bit
                       << ". Number of repetitions: " << retry;
          }
        }
      } // bit loop
    }   // row loop
  };

  auto check_configuration = [this](bool first_stage) {
    bool configurationError = false;

    // Read back the applied configuration (optional)
    auto rawdata = getFrame(true);

    IFLOG(DEBUG) {
      LOG(DEBUG) << "Matrix Stage " << (first_stage ? "1" : "2");
      for(auto& d : frame_decoder_.splitFrame(rawdata)) {
        LOG(DEBUG) << to_bit_string(d);
      }
    }

    auto data = frame_decoder_.decodeFrame(rawdata, false);
    for(const auto& px_cfg : pixelConfiguration) {
      auto address = px_cfg.first;
      auto reading = dynamic_cast<clictd_pixel*>(data[address].get());

      // Compare with value read from the matrix:
      if(first_stage) {
        if(px_cfg.second.first != *reading) {
          LOG(ERROR) << "Matrix configuration (stage 1) of pixel " << static_cast<int>(address.first) << ","
                     << static_cast<int>(address.second) << " does not match:";
          LOG(ERROR) << to_bit_string(px_cfg.second.first.GetLatches()) << " != " << to_bit_string(reading->GetLatches());
          configurationError = true;
        }
      } else {
        if(px_cfg.second.second != *reading) {
          LOG(ERROR) << "Matrix configuration (stage 2) of pixel " << static_cast<int>(address.first) << ","
                     << static_cast<int>(address.second) << " does not match:";
          LOG(ERROR) << to_bit_string(px_cfg.second.second.GetLatches()) << " != " << to_bit_string(reading->GetLatches());
          configurationError = true;
        }
      }
    }

    if(configurationError) {
      throw DataException("Matrix configuration mismatch");
    }
  };

  LOG(INFO) << "Resetting matrix...";
  getFrame(true);

  LOG(INFO) << "Matrix configuration - Stage 1";
  // Write 0x01 to ’configCtrl’ register (start 1st configuration stage)
  this->setRegister("configctrl", 0x01);
  // Check if clock is stopped and also clear readout/clock status register by reading it
  check_clk_stopped();
  // Configure stage 1
  configure_stage(true);
  // Write 0x00 to ’configCtrl’ register - switch back to readout mode.
  this->setRegister("configctrl", 0x00);
  // Check if the clock was restarted
  check_clk_running();

  // Reset the readout FSM and clear the FPGA FIFO
  setMemory("rdcontrol", 2);
  usleep(10);
  check_configuration(true);

  LOG(INFO) << "Matrix configuration - Stage 2";
  // Write 0x02 to ’configCtrl’ register (start 2nd configuration stage)
  this->setRegister("configctrl", 0x02);
  // Check if clock is stopped and also clear readout/clock status register by reading it
  check_clk_stopped();
  // Configure stage 2
  configure_stage(false);
  // Write 0x00 to ’configCtrl’ register - switch back to readout mode.
  this->setRegister("configctrl", 0x00);
  // Check if the clock was restarted
  check_clk_running();

  // Reset the readout FSM and clear the FPGA FIFO
  setMemory("rdcontrol", 2);
  usleep(10);

  check_configuration(false);
  LOG(INFO) << "Verified matrix configuration.";
  // Configuration is complete

  // Switch back compression to what it was:
  setRegister("nocompress", compression);
}

void CLICTDDevice::setSpecialRegister(std::string name, uint32_t value) {
  if(name == "vanalog1" || name == "vthreshold") {
    // 9-bit register, just linearly add:
    uint8_t lsb = value & 0x00FF;
    uint8_t msb = (value >> 8) & 0x01;
    // Set the two values:
    this->setRegister(name + "_msb", msb);
    this->setRegister(name + "_lsb", lsb);
  } else if(name == "configdata") {
    // 16-bit register, just linearly add:
    uint8_t lsb = value & 0x00FF;
    uint8_t msb = (value >> 8) & 0xFF;
    // Set the two values:
    this->setRegister(name + "_msb", msb);
    this->setRegister(name + "_lsb", lsb);
  } else if(name == "longcnt") {
    // Reconfiguring the frame decoder with the new setting:
    frame_decoder_.setLongCounter(static_cast<bool>(value));

    // Resolve name against register dictionary:
    auto reg = _registers.get(name);
    this->process_register_write(reg, value);
  } else if(name == "nocompress") {
    // Resolve name against register dictionary:
    auto reg = _registers.get(name);
    this->process_register_write(reg, value);

    // Flush the matrix:
    LOG(INFO) << "Flushing the matrix to clear hit flags";
    getFrame(true);
  }
}

uint32_t CLICTDDevice::getSpecialRegister(std::string name) {
  uint32_t value = 0;
  if(name == "vanalog1" || name == "vthreshold") {
    // 9-bit register, just linearly add:
    auto lsb = this->getRegister(name + "_lsb");
    uint32_t msb = this->getRegister(name + "_msb");
    // Cpmbine the two values:
    value = ((msb & 0x1) << 8) | (lsb & 0xFF);
  } else if(name == "configdata") {
    // 16-bit register, just linearly add:
    auto lsb = this->getRegister(name + "_lsb");
    uint32_t msb = this->getRegister(name + "_msb");
    // Cpmbine the two values:
    value = ((msb & 0xFF) << 8) | (lsb & 0xFF);
  } else {
    // Well, otherwise just read the register:

    // Resolve name against register dictionary:
    auto reg = _registers.get(name);
    value = this->process_register_read(reg);
  }
  return value;
}

void CLICTDDevice::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << " PWELL: " << _config.Get("pwell", CLICTD_PWELL) << "V";
  this->setVoltage("pwell", _config.Get("pwell", CLICTD_PWELL), _config.Get("pwell_current", CLICTD_PWELL_CURRENT));
  this->switchOn("pwell");

  LOG(DEBUG) << " SUB: " << _config.Get("sub", CLICTD_SUB) << "V";
  this->setVoltage("sub", _config.Get("sub", CLICTD_SUB), _config.Get("sub_current", CLICTD_SUB_CURRENT));
  this->switchOn("sub");

  // Wait a bit
  usleep(1000);

  // Power rails:
  LOG(DEBUG) << " VDDD: " << _config.Get("vddd", CLICTD_VDDD) << "V";
  this->setVoltage("vddd", _config.Get("vddd", CLICTD_VDDD), _config.Get("vddd_current", CLICTD_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(DEBUG) << " VDDA: " << _config.Get("vdda", CLICTD_VDDA) << "V";
  this->setVoltage("vdda", _config.Get("vdda", CLICTD_VDDA), _config.Get("vdda_current", CLICTD_VDDA_CURRENT));
  this->switchOn("vdda");
}

void CLICTDDevice::powerDown() {
  LOG(INFO) << "Power off";

  LOG(DEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(DEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(DEBUG) << "Turn off PWELL";
  this->switchOff("pwell");

  LOG(DEBUG) << "Turn off SUB";
  this->switchOff("sub");

  // Matrix not configured anymore:
  matrixConfigured = false;
}

void CLICTDDevice::daqStart() {
  LOG(INFO) << "DAQ started.";
}

void CLICTDDevice::daqStop() {
  LOG(INFO) << "DAQ stopped.";
}

void CLICTDDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vdda") << "W";

  LOG(INFO) << "PWELL:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("pwell") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("pwell") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("pwell") << "W";

  LOG(INFO) << "SUB:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("sub") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("sub") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("sub") << "W";
}

pearydata CLICTDDevice::getData() {
  auto rawdata = getFrame();
  return frame_decoder_.decodeFrame(rawdata);
}

void CLICTDDevice::setOutputMultiplexer(std::string name) {
  std::map<std::string, int> monitordacsel{{"vbiasresettransistor", 1},
                                           {"vreset", 2},
                                           {"vbiaslevelshift", 3},
                                           {"vanalog1", 4},
                                           {"vanalog1_lsb", 4},
                                           {"vanalog1_msb", 4},
                                           {"vanalog2", 5},
                                           {"vbiaspreampn", 6},
                                           {"vncasc", 7},
                                           {"vpcasc", 8},
                                           {"vfbk", 9},
                                           {"vbiasikrum", 10},
                                           {"vbiasdiscn", 11},
                                           {"vbiasdiscp", 12},
                                           {"vbiasdac", 13},
                                           {"vthreshold", 14},
                                           {"vthreshold_lsb", 14},
                                           {"vthreshold_msb", 14},
                                           {"vncasccomp", 15},
                                           {"vbiaslevelshiftstby", 16},
                                           {"vbiaspreampnstby", 17},
                                           {"vbiasdiscnstby", 18},
                                           {"vbiasdiscpstby", 19},
                                           {"vbiasdacstby", 20},
                                           {"vinternalbandgap", 21}};

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  this->setRegister("monitordacsel", monitordacsel[name]);
}

void CLICTDDevice::triggerPatternGenerator(bool sleep) {

  LOG(DEBUG) << "Triggering pattern generator once.";

  setMemory("wgcontrol", 1);

  // Wait for its length before returning:
  if(sleep) {
    LOG(DEBUG) << "Waiting for pattern generator to finish...";
    size_t total_sleep = 0;
    while(getMemory("wgstatus") & 0x1) {
      usleep(100);
      total_sleep += 100;
      // After three seconds, fail:
      if(total_sleep > 3000000) {
        throw DataException("Pattern generator failed to return within 3 s");
      }
    }
    usleep(100);
  }
}
