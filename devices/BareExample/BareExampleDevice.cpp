/** Caribou bare device example implementation
 */

#include "BareExampleDevice.hpp"
#include "log.hpp"

using namespace caribou;

BareExampleDevice::BareExampleDevice(caribou::Configuration config) : caribouDevice(config) {
  _dispatcher.add("frobicate", &BareExampleDevice::frobicate, this);
  _dispatcher.add("unfrobicate", &BareExampleDevice::unfrobicate, this);
}

BareExampleDevice::~BareExampleDevice() {
  LOG(INFO) << DEVICE_NAME << ": shutdown, delete device";
}

std::string BareExampleDevice::getFirmwareVersion() {
  return "42.23alpha";
}

std::string BareExampleDevice::getName() {
  return DEVICE_NAME;
}

void BareExampleDevice::powerOn() {
  LOG(INFO) << DEVICE_NAME << ": power on";
}

void BareExampleDevice::powerOff() {
  LOG(INFO) << DEVICE_NAME << ": power off";
}

void BareExampleDevice::daqStart() {
  LOG(INFO) << DEVICE_NAME << ": daq started";
}

void BareExampleDevice::daqStop() {
  LOG(INFO) << DEVICE_NAME << ": daq stopped";
}

std::vector<uint32_t> BareExampleDevice::getRawData() {
  return {0u, 2u, 4u, 1u, 3u, 5u};
}

pearydata BareExampleDevice::getData() {
  pearydata x;
  x[{0u, 0u}] = std::make_unique<pixel>();
  x[{8u, 16u}] = std::make_unique<pixel>();
  return x;
}

std::vector<std::pair<std::string, uint32_t>> BareExampleDevice::getRegisters() {
  return {{"reg0", 0u}, {"reg1", 1u}};
}

std::vector<uint64_t> BareExampleDevice::timestampsPatternGenerator() {
  return {0u, 1u};
}

// custom device functionality exported via the dispatcher

int32_t BareExampleDevice::frobicate(int32_t a) {
  return (a << 2) & 0b010101010101010101010101010101;
}

std::string BareExampleDevice::unfrobicate(int32_t b) {
  return "blub" + std::to_string(b);
}
