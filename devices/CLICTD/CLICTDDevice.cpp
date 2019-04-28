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
