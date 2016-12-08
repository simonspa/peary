/**
 * Caribou Example Device implementation
 */

#include "example.h"
#include "hal.h"
#include "log.h"

using namespace caribou;

example::~example() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

void example::init() {
  LOG(logINFO) << DEVICE_NAME << ": Initialized.";
}

void example::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Power on.";
}

void example::powerOff() {
  LOG(logINFO) << DEVICE_NAME << ": Power off.";
}

void example::daqStart() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ started.";
}

void example::daqStop() {
  LOG(logINFO) << DEVICE_NAME << ": DAQ stopped.";
}

caribouDevice* caribou::generator(caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  example* mDevice = new example(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
