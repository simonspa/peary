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

void example::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Power on.";

  // Vectors can be read directly from the config and passed to an interface
  //_hal->sendCommand(12,_config.Get("some-spi-data",std::vector<uint8_t>()));

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

void example::exampleCall() {
  LOG(logINFO) << DEVICE_NAME << ": exampleCall";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  example* mDevice = new example(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
