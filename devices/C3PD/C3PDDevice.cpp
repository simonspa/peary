/**
 * Caribou implementation for the C3PD
 */

#include "C3PDDevice.hpp"
#include "hal.hpp"
#include "log.hpp"

#include <fstream>

using namespace caribou;

C3PDDevice::C3PDDevice(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), C3PD_DEFAULT_I2C) {

  _dispatcher.add("powerStatusLog", &C3PDDevice::powerStatusLog, this);

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);
  _periphery.add("vdda", PWR_OUT_6);

  _periphery.add("ain", BIAS_1);
  _periphery.add("ref", BIAS_2);

  _periphery.add("aout", VOL_IN_2);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(C3PD_REGISTERS);

  // Map C3PD control
  void* control_base = _hal->getMappedMemoryRW(C3PD_CONTROL_BASE_ADDRESS, C3PD_CONTROL_MAP_SIZE, C3PD_CONTROL_MAP_MASK);

  // set default C3PD control
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + C3PD_RESET_OFFSET);
  *control_reg = 0; // keep C3PD in reset state
}

void C3PDDevice::configure() {
  LOG(INFO) << "Configuring";
  reset();

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void C3PDDevice::configureMatrix(std::string filename) {

  // Testpulses can only be enabled in full rows and columns:
  std::map<int, uint8_t> test_columns;
  std::map<int, uint8_t> test_rows;

  LOG(DEBUG) << "Reading pixel matrix file.";
  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    LOG(ERROR) << "Could not open matrix file \"" << filename << "\"";
    throw ConfigInvalid("Could not open matrix file \"" + filename + "\"");
  }

  std::string line = "";
  while(std::getline(pxfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pxline(line);
    int column, row, tpenable, dummy;
    if(pxline >> row >> column >> dummy >> dummy >> dummy >> tpenable >> dummy) {
      if(static_cast<bool>(tpenable)) {
        test_columns[column / 8] |= (1 << (column % 8));
        // BEWARE: C3PD is flipped wrt CLICpix2 in rows!
        test_rows[15 - row / 8] |= (1 << (7 - (row % 8)));
      }
    }
  }

  std::string alphabet("abcdefghijklmnop");
  for(const auto& c : test_columns) {
    LOG(DEBUG) << "Column reg " << c.first << " bits " << to_bit_string(c.second);
    std::string reg("tpce");
    reg += alphabet.at(c.first);
    this->setRegister(reg, c.second);
  }

  for(const auto& r : test_rows) {
    LOG(DEBUG) << "Row reg " << r.first << " bits " << to_bit_string(r.second);
    std::string reg("tpre");
    reg += alphabet.at(r.first);
    this->setRegister(reg, r.second);
  }
}

void C3PDDevice::reset() {
  LOG(DEBUG) << "Resetting";

  void* control_base = _hal->getMappedMemoryRW(C3PD_CONTROL_BASE_ADDRESS, C3PD_CONTROL_MAP_SIZE, C3PD_CONTROL_MAP_MASK);
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + C3PD_RESET_OFFSET);
  *control_reg &= ~(C3PD_CONTROL_RESET_MASK); // assert reset
  usleep(1);
  *control_reg |= C3PD_CONTROL_RESET_MASK; // deny reset
}

C3PDDevice::~C3PDDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

std::string C3PDDevice::getName() {
  return DEVICE_NAME;
}

void C3PDDevice::powerUp() {
  LOG(INFO) << "Powering up";

  // Power rails:
  LOG(DEBUG) << " VDDD: " << _config.Get("vddd", C3PD_VDDD) << "V";
  this->setVoltage("vddd", _config.Get("vddd", C3PD_VDDD), _config.Get("vddd_current", C3PD_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(DEBUG) << " VDDA: " << _config.Get("vdda", C3PD_VDDA) << "V";
  this->setVoltage("vdda", _config.Get("vdda", C3PD_VDDA), _config.Get("vdda_current", C3PD_VDDA_CURRENT));
  this->switchOn("vdda");

  // Bias voltages:
  LOG(DEBUG) << " Reference voltage: " << _config.Get("ref", C3PD_REF) << "V";
  this->setVoltage("ref", _config.Get("ref", C3PD_REF));
  this->switchOn("ref");

  LOG(DEBUG) << " Analog-In: " << _config.Get("ain", C3PD_AIN) << "V";
  this->setVoltage("ain", _config.Get("ain", C3PD_AIN));
  this->switchOn("ain");
}

void C3PDDevice::powerDown() {
  LOG(INFO) << "Power off";

  LOG(DEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(DEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(DEBUG) << "Turn off AIN";
  this->switchOff("ain");

  LOG(DEBUG) << "Turn off REF";
  this->switchOff("ref");
}

void C3PDDevice::daqStart() {
  LOG(INFO) << "DAQ started.";
}

void C3PDDevice::daqStop() {
  LOG(INFO) << "DAQ stopped.";
}

void C3PDDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vdda") << "W";
}
