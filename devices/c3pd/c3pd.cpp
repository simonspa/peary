/**
 * Caribou implementation for the C3PD
 */

#include "c3pd.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

C3PD::C3PD(const caribou::Configuration config) : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), C3PD_DEFAULT_I2C) {

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

void C3PD::configure() {
  LOG(logINFO) << "Configuring " << DEVICE_NAME;
  reset();

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void C3PD::reset() {
  LOG(logDEBUG) << "Resetting " << DEVICE_NAME;

  void* control_base = _hal->getMappedMemoryRW(C3PD_CONTROL_BASE_ADDRESS, C3PD_CONTROL_MAP_SIZE, C3PD_CONTROL_MAP_MASK);
  volatile uint32_t* control_reg =
    reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::intptr_t>(control_base) + C3PD_RESET_OFFSET);
  *control_reg &= ~(C3PD_CONTROL_RESET_MASK); // assert reset
  usleep(1);
  *control_reg |= C3PD_CONTROL_RESET_MASK; // deny reset
}

C3PD::~C3PD() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string C3PD::getName() {
  return DEVICE_NAME;
}

void C3PD::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up C3PD";

  // Power rails:
  LOG(logDEBUG) << " VDDD: " << _config.Get("vddd", C3PD_VDDD) << "V";
  this->setVoltage("vddd", _config.Get("vddd", C3PD_VDDD), _config.Get("vddd_current", C3PD_VDDD_CURRENT));
  this->switchOn("vddd");

  LOG(logDEBUG) << " VDDA: " << _config.Get("vdda", C3PD_VDDA) << "V";
  this->setVoltage("vdda", _config.Get("vdda", C3PD_VDDA), _config.Get("vdda_current", C3PD_VDDA_CURRENT));
  this->switchOn("vdda");

  // Bias voltages:
  LOG(logDEBUG) << " Reference voltage: " << _config.Get("ref", C3PD_REF) << "V";
  this->setVoltage("ref", _config.Get("ref", C3PD_REF));
  this->switchOn("ref");

  LOG(logDEBUG) << " Analog-In: " << _config.Get("ain", C3PD_AIN) << "V";
  this->setVoltage("ain", _config.Get("ain", C3PD_AIN));
  this->switchOn("ain");
}

void C3PD::powerDown() {
  LOG(logINFO) << DEVICE_NAME << ": Power off C3PD";

  LOG(logDEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(logDEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(logDEBUG) << "Turn off AIN";
  this->switchOff("ain");

  LOG(logDEBUG) << "Turn off REF";
  this->switchOff("ref");
}

void C3PD::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void C3PD::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

void C3PD::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

  LOG(logINFO) << "VDDD:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(logINFO) << "VDDA:";
  LOG(logINFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(logINFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(logINFO) << "\tBus power  : " << this->getPower("vdda") << "W";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  C3PD* mDevice = new C3PD(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
