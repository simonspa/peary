/**
 * Caribou implementation for the CCPDV5
 */

#include "ccpdv5.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

CCPDV5::CCPDV5(const caribou::Configuration config) : pearyDevice(config, "/dev/i2c-8") {

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);

  // Add the register definitions to the dictionary for convenient lookup of names:
  //_registers.add(CCPDV5_REGISTERS);
}

void CCPDV5::configure() {
  LOG(logINFO) << "Configuring " << DEVICE_NAME;
  reset();

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void CCPDV5::reset() {
  LOG(logDEBUG) << "Resetting " << DEVICE_NAME;
}

CCPDV5::~CCPDV5() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string CCPDV5::getName() {
  return DEVICE_NAME;
}

void CCPDV5::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Powering up CCPDV5";

}

void CCPDV5::powerDown() {
}

void CCPDV5::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void CCPDV5::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

void CCPDV5::powerStatusLog() {
  LOG(logINFO) << DEVICE_NAME << " power status:";

}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  CCPDV5* mDevice = new CCPDV5(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
