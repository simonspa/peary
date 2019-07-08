/**
 * Caribou MCP9804Device Device implementation
 */

#include <string>

#include "MCP9804Device.hpp"
#include "carboard/Carboard.hpp"
#include "utils/log.hpp"

using namespace caribou;

/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH BUS_I2C2

/** Default I2C address for CLICTD chip-board with unconnected I2C address lines
 */
#define MCP9804_DEFAULT_I2C 0x18

MCP9804Device::MCP9804Device(const caribou::Configuration config)
    : AuxiliaryDevice(config, std::string(DEFAULT_DEVICEPATH), MCP9804_DEFAULT_I2C) {
  _config = config;

  _dispatcher.add("getTemperature", &MCP9804Device::getTemperature, this);

  const unsigned int addr = 0x06;
  auto manufacturer_id = AuxiliaryDevice<iface_i2c>::receive(addr, 2);
  LOG(INFO) << "Manufacturer ID:\t" << to_hex_string(uint16_t((manufacturer_id.front() << 8) | manufacturer_id.back()));
  auto device_info = AuxiliaryDevice<iface_i2c>::receive(addr, 2);
  LOG(INFO) << "Device ID:\t" << to_hex_string(device_info.front()) << "\nDevice Revision:\t"
            << to_hex_string(device_info.back());
}

MCP9804Device::~MCP9804Device() {}

double MCP9804Device::readTemperature() {
  const unsigned int temp_reg = 0x5;
  auto temps = AuxiliaryDevice<iface_i2c>::receive(temp_reg, 2);

  double temperature = 0.0;
  if(temps.front() & 0x10) {
    temperature = 256. - (static_cast<double>(temps.front() & 0x0F) * 16. + static_cast<double>(temps.back()) / 16.);
  } else {
    temperature = (static_cast<double>(temps.front() & 0x0F) * 16. + static_cast<double>(temps.back()) / 16.);
  }
  return temperature;
}

void MCP9804Device::getTemperature() {
  LOG(INFO) << "Temperature: " << readTemperature() << " C";
}

pearydata MCP9804Device::getData() {
  return pearydata();
}
