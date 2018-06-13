/**
 * Caribou implementation for the CCPDV5
 */

#include "CCPDv5Device.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

CCPDv5Device::CCPDv5Device(const caribou::Configuration config) : pearyDevice(config, "/dev/i2c-8") {

  // Set up periphery
  _periphery.add("vddd", PWR_OUT_2);

  // Add the register definitions to the dictionary for convenient lookup of names:
  //_registers.add(CCPDV5_REGISTERS);
}

void CCPDv5Device::configure() {
  LOG(INFO) << "Configuring " << DEVICE_NAME;
  reset();

  // Call the base class configuration function:
  pearyDevice<iface_i2c>::configure();
}

void CCPDv5Device::reset() {
  LOG(DEBUG) << "Resetting " << DEVICE_NAME;
}

CCPDv5Device::~CCPDv5Device() {
  LOG(INFO) << DEVICE_NAME << ": Shutdown, delete device.";
  powerOff();
}

std::string CCPDv5Device::getName() {
  return DEVICE_NAME;
}

void CCPDv5Device::powerUp() {
  LOG(INFO) << DEVICE_NAME << ": Powering up CCPDv5Device";
}

void CCPDv5Device::powerDown() {}

void CCPDv5Device::daqStart() {
  LOG(INFO) << DEVICE_NAME << ": DAQ started.";
}

void CCPDv5Device::daqStop() {
  LOG(INFO) << DEVICE_NAME << ": DAQ stopped.";
}

void CCPDv5Device::powerStatusLog() {
  LOG(INFO) << DEVICE_NAME << " power status:";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(DEBUG) << "Generator: " << DEVICE_NAME;
  CCPDv5Device* mDevice = new CCPDv5Device(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
