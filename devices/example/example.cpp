/**
 * Caribou Example Device implementation
 */

#include "example.hpp"
#include "loopback.hpp"
#include "hal.hpp"
#include "log.hpp"

using namespace caribou;

example::~example() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

std::string example::getName() { return DEVICE_NAME; }

void example::powerOn() {
  LOG(logINFO) << DEVICE_NAME << ": Power on.";

  // Read a DAC value from the config if it exists, otherwise take default
  uint32_t dac_test = _config.Get("dac_test",static_cast<uint32_t>(EXAMPLE_DAC_TEST));
  LOG(logDEBUG) << DEVICE_NAME << " config sets DAC_TEST=" << dac_test;

  LOG(logINFO) << "ADDR from dict: " << (int)exampleDict.getAddress("vthreshold");
  std::vector<std::string> names = exampleDict.getNames();
  for(auto i : names) {
    LOG(logDEBUG) << i;
  }
  
  // Vectors can be read directly from the config and passed to an interface
  _hal->send(exampleDict.getAddress("vthreshold"),_config.Get("sample-registers",std::vector<uint8_t>{EXAMPLE_DAC_VEC}));
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
  // Vectors can be read directly from the config and passed to an interface
  _hal->send(_config.Get("sample-registers",std::vector<uint8_t>{EXAMPLE_DAC_VEC}));
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  example* mDevice = new example(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
