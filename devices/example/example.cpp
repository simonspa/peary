/**
 * Caribou Example Device implementation
 */

#include "example.hpp"
#include "hal.hpp"
#include "log.hpp"
#include "loopback.hpp"

using namespace caribou;

example::~example() {
  LOG(logINFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

std::string example::getName() {
  return DEVICE_NAME;
}

void example::powerUp() {
  LOG(logINFO) << DEVICE_NAME << ": Power on.";

  // Read a DAC value from the config if it exists, otherwise take default
  uint32_t dac_test = _config.Get("dac_test", static_cast<uint32_t>(EXAMPLE_DAC_TEST));
  LOG(logDEBUG) << DEVICE_NAME << " config sets DAC_TEST=" << dac_test;

  LOG(logINFO) << "Register from dictionary: " << _registers.get("vthreshold");
  LOG(logINFO) << "Register value from config: " << _config.Get("vthreshold", EXAMPLE_DAC_TEST);
  this->setRegister("vthreshold", _config.Get("dac_test", EXAMPLE_DAC_TEST));

  // Vectors can be read directly from the config and passed to an interface
  LOG(logINFO) << listVector(_config.Get("sample_registers", std::vector<uint8_t>{EXAMPLE_DAC_VEC}));

  try {
    LOG(logINFO) << "Register value from config without default: " << _config.Get<uint8_t>("vthr");
  } catch(ConfigMissingKey& e) {
    LOG(logERROR) << e.what();
  }
}

void example::powerDown() {
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
  _hal->send(_config.Get("sample-registers", std::vector<uint8_t>{EXAMPLE_DAC_VEC}));
}

void example::setSpecialRegister(std::string name, uint32_t value) {

  LOG(logDEBUG) << "Treating special register \"" << name << "\"";
}

caribouDevice* caribou::generator(const caribou::Configuration config) {
  LOG(logDEBUG) << "Generator: " << DEVICE_NAME;
  example* mDevice = new example(config);
  return dynamic_cast<caribouDevice*>(mDevice);
}
