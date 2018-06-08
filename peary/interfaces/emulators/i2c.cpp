/**
 * Caribou I2C interface emulator
 */

#include <cerrno>
#include <cstring>

#include "log.hpp"
#include "utils.hpp"

#include "i2c.hpp"

using namespace caribou;

iface_i2c::iface_i2c(std::string const& device_path) : Interface(device_path) {
  LOG(TRACE) << "Opened I2C device at " << device_path;
}

iface_i2c::~iface_i2c() {}

void iface_i2c::setAddress(i2c_address_t const address) {
  LOG(TRACE) << "Talking to I2C slave at address " << to_hex_string(address);
}

i2c_t iface_i2c::write(const i2c_t& address, const i2c_t& data) {
  std::lock_guard<std::mutex> lock(mutex);

  setAddress(address);

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(address) << ": Writing data \""
             << to_hex_string(data) << "\"";

  return 0;
}

std::pair<i2c_reg_t, i2c_t> iface_i2c::write(const i2c_t& address, const std::pair<i2c_reg_t, i2c_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  setAddress(address);

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(address) << ": Register "
             << to_hex_string(data.first) << " Writing data \"" << to_hex_string(data.second) << "\"";

  return std::make_pair(0, 0);
}

std::vector<i2c_t> iface_i2c::write(const i2c_t& address, const i2c_t& reg, const std::vector<i2c_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  setAddress(address);

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(address) << ": Register " << to_hex_string(reg)
             << "\n\t Writing block data: \"" << listVector(data, ", ", true) << "\"";

  return std::vector<i2c_t>();
}

std::vector<i2c_t> iface_i2c::read(const i2c_t& address, const unsigned int length) {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;

  if(length != 1)
    throw CommunicationError("Read operation of multiple data from the device without internal registers is not possible");

  setAddress(address);

  data.resize(1);

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(address) << ": Read data \""
             << to_hex_string(data[0]) << "\"";

  return data;
}

std::vector<i2c_t> iface_i2c::read(const i2c_t& address, const i2c_t reg, const unsigned int length) {

  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;

  setAddress(address);

  data.resize(length);

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(address) << ": Register " << to_hex_string(reg)
             << "\n\t Read block data \"" << listVector(data, ", ", true) << "\"";

  return data;
}

std::vector<i2c_t> iface_i2c::wordwrite(const i2c_t&, const uint16_t&, const std::vector<i2c_t>&) {
  return std::vector<i2c_t>();
}

std::vector<i2c_t> iface_i2c::wordread(const i2c_t&, const uint16_t, const unsigned int) {
  return std::vector<i2c_t>();
}
