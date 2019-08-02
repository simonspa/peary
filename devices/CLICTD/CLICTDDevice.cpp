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

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);
  _periphery.add("vdda", PWR_OUT_6);
  _periphery.add("pwell", PWR_OUT_8);
  _periphery.add("sub", PWR_OUT_3);

  _periphery.add("analog_out", VOL_IN_1);
  _periphery.add("dac_out", VOL_IN_1);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(CLICTD_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(CLICPIX2_MEMORY);

  // set default CLICpix2 control
  setMemory("reset", 0);
}

void CLICTDDevice::configure() {
  LOG(INFO) << "Configuring";
  configureClock(_config.Get<bool>("clock_internal", true));
  reset();
  mDelay(10);

  // Call the base class configuration function:
  CaribouDevice<iface_i2c>::configure();
}

void CLICTDDevice::reset() {
  LOG(DEBUG) << "Resetting";

  // assert reset:
  setMemory("reset", getMemory("reset") & ~(CLICPIX2_CONTROL_RESET_MASK));
  usleep(1);
  // deny reset:
  setMemory("reset", getMemory("reset") | CLICPIX2_CONTROL_RESET_MASK);
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
    int column, row, mask, tp_dig, tp_ana0, tp_ana1, tp_ana2, tp_ana3, tp_ana4, tp_ana5, tp_ana6, tp_ana7;
    int threshold0, threshold1, threshold2, threshold3, threshold4, threshold5, threshold6, threshold7;
    if(pxline >> column >> row >> mask >> tp_dig >> tp_ana0 >> tp_ana1 >> tp_ana2 >> tp_ana3 >> tp_ana4 >> tp_ana5 >>
       tp_ana6 >> tp_ana7 >> threshold0 >> threshold1 >> threshold2 >> threshold3 >> threshold4 >> threshold5 >>
       threshold6 >> threshold7) {

      // Prepare analog testpulse bits:
      uint8_t tp_analog = ((tp_ana7 & 0x1) << 7) | ((tp_ana6 & 0x1) << 6) | ((tp_ana5 & 0x1) << 5) | ((tp_ana4 & 0x1) << 4) |
                          ((tp_ana3 & 0x1) << 3) | ((tp_ana2 & 0x1) << 2) | ((tp_ana1 & 0x1) << 1) | (tp_ana0 & 0x1);

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
        std::make_pair(pixelConfigStage1(mask, tp_dig, tp_analog, thresholds), pixelConfigStage2(thresholds));
      if(mask)
        masked++;
    }
  }
  LOG(INFO) << pixelsConfig.size() << " pixel configurations cached, " << masked << " of which are masked";
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
      LOG(INFO) << "Verified matrix configuration.";
      break;
    } catch(caribou::DataException& e) {
      LOG(ERROR) << e.what();
      if(++retry == retry_max) {
        throw CommunicationError("Matrix configuration failed");
      }
      LOG(INFO) << "Repeating configuration attempt";
    }
  }
}

void CLICTDDevice::programMatrix() {
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

  LOG(INFO) << "Matrix configuration - Stage 1";
  // Write 0x01 to ’configCtrl’ register (start 1st configuration stage)
  this->setRegister("configctrl", 0x01);
  // For each of the pixels per column, do
  for(size_t row = 0; row < 128; row++) {
    // Read configuration bits for STAGE 1 one by one:
    for(size_t bit = 22; bit > 0; bit--) {
      auto value = bitvalues(pixelConfiguration, true, row, bit - 1);
      // Load ’configData’ register with bit 21 of the 1st configuration stage (1 bit per column)
      this->setRegister("configdata", value);
      LOG(DEBUG) << "Row " << row << ", bit " << (bit - 1) << ": " << to_bit_string(value);
      // Write 0x11 to ’configCtrl’ register to shift configuration in the matrix
      this->setRegister("configctrl", 0x11);
      // Write 0x01 to ’configCtrl’ register
      this->setRegister("configctrl", 0x01);
    }
  }
  // Write 0x00 to ’configCtrl’ register
  this->setRegister("configctrl", 0x00);

  // Read back the applied configuration (optional)

  LOG(INFO) << "Matrix configuration - Stage 2";
  // Write 0x02 to ’configCtrl’ register (start 2nd configuration stage)
  this->setRegister("configctrl", 0x02);
  // For each of the pixels per column, do
  for(size_t row = 0; row < 128; row++) {
    // Read configuration bits for STAGE 1 one by one:
    for(size_t bit = 22; bit > 0; bit--) {
      auto value = bitvalues(pixelConfiguration, false, row, bit - 1);
      // Load ’configData’ register with bit 21 of the 2nd configuration stage (1 bit per column)
      this->setRegister("configdata", value);
      LOG(DEBUG) << "Row " << row << ", bit " << (bit - 1) << ": " << to_bit_string(value);
      // Write 0x12 to ’configCtrl’ register to shift configuration in the matrix
      this->setRegister("configctrl", 0x12);
      // Write 0x02 to ’configCtrl’ register
      this->setRegister("configctrl", 0x02);
    }
  }
  // Write 0x00 to ’configCtrl’ register
  this->setRegister("configctrl", 0x00);

  // Read back the applied configuration (optional)

  // Configuration is complete
}

void CLICTDDevice::setSpecialRegister(std::string name, uint32_t value) {
  if(name == "vanalog1" || name == "vthreshold") {
    // 9-bit register, just linearly add:
    uint8_t lsb = value & 0x00FF;
    uint8_t msb = (value >> 8) & 0x01;
    // Set the two values:
    this->setRegister(name + "_msb", msb);
    this->setRegister(name + "_lsb", lsb);
  }
}

uint32_t CLICTDDevice::getSpecialRegister(std::string name) {
  uint32_t value = 0;
  if(name == "vanalog1" || name == "vthreshold") {
    // 9-bit register, just linearly add:
    auto lsb = this->getRegister(name + "_lsb");
    auto msb = this->getRegister(name + "_msb");
    // Cpmbine the two values:
    value = ((msb & 0x1) << 8) || (lsb & 0xFF);
  }

  return value;
}

void CLICTDDevice::powerUp() {
  LOG(INFO) << "Powering up";

  // Power rails:
  LOG(DEBUG) << " VDDD: " << _config.Get("vddd", CLICTD_VDDD) << "V";
  this->setVoltage("vddd", _config.Get("vddd", CLICTD_VDDD), _config.Get("vddd_current", CLICTD_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(DEBUG) << " VDDA: " << _config.Get("vdda", CLICTD_VDDA) << "V";
  this->setVoltage("vdda", _config.Get("vdda", CLICTD_VDDA), _config.Get("vdda_current", CLICTD_VDDA_CURRENT));
  this->switchOn("vdda");

  LOG(DEBUG) << " PWELL: " << _config.Get("pwell", CLICTD_PWELL) << "V";
  this->setVoltage("pwell", _config.Get("pwell", CLICTD_PWELL), _config.Get("pwell_current", CLICTD_PWELL_CURRENT));
  this->switchOn("pwell");

  LOG(DEBUG) << " SUB: " << _config.Get("sub", CLICTD_SUB) << "V";
  this->setVoltage("sub", _config.Get("sub", CLICTD_SUB), _config.Get("sub_current", CLICTD_SUB_CURRENT));
  this->switchOn("sub");
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
  pearydata decoded;

  auto rawdata = getRawData();

  bool frame_started = false;
  uint8_t column = 0;
  for(auto data : rawdata) {
    // Check for header:
    if((data << 8) & 0x3FFF) { // FIXME
      if((data & 0xFF) == 0xA8) {
        LOG(DEBUG) << "Header: Frame start";
        frame_started = true;
      } else if((data & 0xFF) == 0x94) {
        LOG(DEBUG) << "Header: Frame end";
        frame_started = false;
      } else {
        column = (data >> 2) & 0xF;
        LOG(DEBUG) << "Header: Column " << column;
      }

      // No header but pixel data.
    }
    // FRAMESTART header, 22bit
    // 0x3FFF-A8

    // FRAMEEND header, 22bit
    // 0x3FFF-94

    // COLUMN header, 22bit
    // 0x3FFF-?
  }

  return decoded;
}

std::vector<uint32_t> CLICTDDevice::getRawData() {
  return std::vector<uint32_t>();
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
