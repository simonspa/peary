/**
 * Caribou Example Device implementation
 */

#include "example.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

example::~example() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

void example::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Power on.";

  // Read a DAC value from the config if it exists, otherwise take default
  uint32_t dac_test = _config.Get("dac_test",static_cast<uint32_t>(EXAMPLE_DAC_TEST));
  LOG(logDEBUG) << DEVICE_NAME << " config sets DAC_TEST=" << dac_test;
  
  // Vectors can be read directly from the config and passed to an interface
  //_hal->sendCommand(12,_config.Get("sample-registers",std::vector<uint8_t>{EXAMPLE_DAC_VEC}));
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
