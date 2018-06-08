/**
 * Caribou Example Device implementation
 */

#include "ExampleDevice.hpp"
#include "hal.hpp"
#include "log.hpp"
#include "loopback.hpp"

using namespace caribou;

ExampleDevice::ExampleDevice(const caribou::Configuration config)
    : pearyDevice(config, std::string(DEFAULT_DEVICEPATH), DEFAULT_ADDRESS) {
  _periphery.add("vd", PWR_OUT_1);
  _registers.add(EXAMPLE_REGISTERS);
  _dispatcher.add("do", &ExampleDevice::doDeviceSpecificThings, this);
  _dispatcher.add("exampleCall", &ExampleDevice::exampleCall, this);
};

ExampleDevice::~ExampleDevice() {
  LOG(INFO) << DEVICE_NAME << ": Shutdown, delete device.";
}

std::string ExampleDevice::getName() {
  return DEVICE_NAME;
}

void ExampleDevice::doDeviceSpecificThings(std::string arg1, int arg2) {
  LOG(INFO) << DEVICE_NAME << ": Calling specific function throug dispatcher.";
  LOG(INFO) << "  input arg1:   " << arg1;
  LOG(INFO) << "  input arg2*2: " << arg2 * 2;
}

void ExampleDevice::powerUp() {
  LOG(INFO) << DEVICE_NAME << ": Power on.";

  // Read a DAC value from the config if it exists, otherwise take default
  uint32_t dac_test = _config.Get("dac_test", static_cast<uint32_t>(EXAMPLE_DAC_TEST));
  LOG(DEBUG) << DEVICE_NAME << " config sets DAC_TEST=" << dac_test;

  LOG(INFO) << "Register from dictionary: " << _registers.get("vthreshold");
  LOG(INFO) << "Register value from config: " << _config.Get("vthreshold", EXAMPLE_DAC_TEST);
  this->setRegister("vthreshold", _config.Get("dac_test", EXAMPLE_DAC_TEST));

  // Vectors can be read directly from the config and passed to an interface
  LOG(INFO) << listVector(_config.Get("sample_registers", std::vector<uint8_t>{EXAMPLE_DAC_VEC}));

  try {
    LOG(INFO) << "Register value from config without default: " << _config.Get<uint8_t>("vthr");
  } catch(ConfigMissingKey& e) {
    LOG(ERROR) << e.what();
  }
}

void ExampleDevice::powerDown() {
  LOG(INFO) << DEVICE_NAME << ": Power off.";
}

void ExampleDevice::daqStart() {
  LOG(INFO) << DEVICE_NAME << ": DAQ started.";
}

void ExampleDevice::daqStop() {
  LOG(INFO) << DEVICE_NAME << ": DAQ stopped.";
}

void ExampleDevice::exampleCall() {
  LOG(INFO) << DEVICE_NAME << ": ExampleCall";
  // Vectors can be read directly from the config and passed to an interface
  _hal->send(_config.Get("sample-registers", std::vector<uint8_t>{EXAMPLE_DAC_VEC}));
}

void ExampleDevice::setSpecialRegister(std::string name, uint32_t) {

  LOG(DEBUG) << "Treating special register \"" << name << "\"";
}
