/** Caribou bare device example implementation
 */

#include "ExampleCaribouDevice.hpp"
#include "log.hpp"

using namespace caribou;

ExampleCaribouDevice::ExampleCaribouDevice(caribou::Configuration config) : pearyDevice(config, "/dev/null") {
  _dispatcher.add("frobicate", &ExampleCaribouDevice::frobicate, this);
  _dispatcher.add("unfrobicate", &ExampleCaribouDevice::unfrobicate, this);
}

ExampleCaribouDevice::~ExampleCaribouDevice() {
  LOG(INFO) << DEVICE_NAME << ": shutdown, delete device";
}

std::string ExampleCaribouDevice::getFirmwareVersion() {
  return "42.23alpha";
}

std::string ExampleCaribouDevice::getName() {
  return DEVICE_NAME;
}

void ExampleCaribouDevice::powerUp() {
  LOG(INFO) << DEVICE_NAME << ": power on";
}

void ExampleCaribouDevice::powerDown() {
  LOG(INFO) << DEVICE_NAME << ": power off";
}

void ExampleCaribouDevice::daqStart() {
  LOG(INFO) << DEVICE_NAME << ": daq started";
}

void ExampleCaribouDevice::daqStop() {
  LOG(INFO) << DEVICE_NAME << ": daq stopped";
}

std::vector<uint32_t> ExampleCaribouDevice::getRawData() {
  return {0u, 2u, 4u, 1u, 3u, 5u};
}

pearydata ExampleCaribouDevice::getData() {
  pearydata x;
  x[{0u, 0u}] = std::make_unique<pixel>();
  x[{8u, 16u}] = std::make_unique<pixel>();
  return x;
}

std::vector<std::pair<std::string, uint32_t>> ExampleCaribouDevice::getRegisters() {
  return {{"reg0", 0u}, {"reg1", 1u}};
}

// custom device functionality exported via the dispatcher

int32_t ExampleCaribouDevice::frobicate(int32_t a) {
  return (a << 2) & 0b010101010101010101010101010101;
}

std::string ExampleCaribouDevice::unfrobicate(int32_t b) {
  return "blub" + std::to_string(b);
}
