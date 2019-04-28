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

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);
  _periphery.add("vdda", PWR_OUT_6);
  _periphery.add("pwell", PWR_OUT_8);
  _periphery.add("sub", PWR_OUT_3);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(CLICTD_REGISTERS);
}

void CLICTDDevice::configure() {
  LOG(INFO) << "Configuring";
  reset();

  // Call the base class configuration function:
  CaribouDevice<iface_i2c>::configure();
}

void CLICTDDevice::reset() {
  LOG(DEBUG) << "Resetting";
}

CLICTDDevice::~CLICTDDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void CLICTDDevice::configureMatrix(std::string filename) {

  if(!filename.empty()) {
    LOG(DEBUG) << "Configuring the pixel matrix from file \"" << filename << "\"";
    // pixelsConfig = readMatrix(filename);
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
  auto bitvalues = [&](size_t row, size_t bit) {
    uint16_t bits = 0;
    for(uint8_t column = 0; column < 16; column++) {
      bool value = pixelConfiguration[std::make_pair(column, row)].GetBit(bit);
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
      auto value = bitvalues(row, bit - 1);
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
    for(size_t bit = 43; bit > 21; bit--) {
      auto value = bitvalues(row, bit);
      // Load ’configData’ register with bit 21 of the 2nd configuration stage (1 bit per column)
      this->setRegister("configdata", value);
      LOG(DEBUG) << "Row " << row << ", bit " << (bit - 22) << ": " << to_bit_string(value);
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
