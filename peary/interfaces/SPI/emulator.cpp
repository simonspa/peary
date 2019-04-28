/**
 * Caribou SPI interface class implementation
 */

#include <climits>
#include <cstring>
#include <utility>

#include "spi.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

using namespace caribou;

iface_spi::iface_spi(std::string const& device_path) : Interface(device_path) {
  std::lock_guard<std::mutex> lock(mutex);

  // Open device
}

iface_spi::~iface_spi() {}

std::pair<spi_reg_t, spi_t> iface_spi::write(const spi_address_t& address, const std::pair<spi_reg_t, spi_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);
  std::pair<spi_reg_t, spi_t> rx = std::make_pair(spi_reg_t(), spi_t());

  LOG(TRACE) << "SPI (" << devicePath() << ") address " << to_hex_string(address) << ": Register "
             << to_hex_string(data.first) << " Wrote data \"" << to_hex_string(data.second) << "\" Read data \""
             << to_hex_string(rx.second) << "\"";

  return rx;
}

std::vector<spi_t> iface_spi::write(const spi_address_t& address, const spi_t& reg, const std::vector<spi_t>& data) {

  std::vector<std::pair<spi_reg_t, spi_t>> _data;
  std::vector<spi_t> rx;

  _data.reserve(data.size());
  for(auto i : data)
    _data.push_back(std::make_pair(reg, i));

  _data = write(address, _data);

  rx.reserve(data.size());
  for(auto i : _data)
    rx.push_back(i.second);

  return rx;
}

std::vector<std::pair<spi_reg_t, spi_t>> iface_spi::write(const spi_address_t& address,
                                                          const std::vector<std::pair<spi_reg_t, spi_t>>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  std::vector<uint8_t> _data((sizeof(spi_reg_t) + sizeof(spi_t)) * data.size());
  std::vector<std::pair<spi_reg_t, spi_t>> rx;

  // pack
  for(struct {
        unsigned int i = 0;
        unsigned int pos = 0;
      } loop;
      loop.i < data.size();
      ++loop.i) {
    std::memcpy(_data.data() + loop.pos, &data[loop.i].second, sizeof(spi_t));
    loop.pos += sizeof(spi_t);
    std::memcpy(_data.data() + loop.pos + sizeof(spi_t), &data[loop.i].first, sizeof(spi_reg_t));
    loop.pos += sizeof(spi_reg_t);
  }

  // unpack
  rx.reserve(data.size());
  for(struct {
        unsigned int i = 0;
        unsigned int pos = 0;
      } loop;
      loop.i < data.size();
      ++loop.i) {
    rx.push_back(std::make_pair(*static_cast<spi_reg_t*>(_data.data() + loop.pos + sizeof(spi_t)),
                                *static_cast<spi_t*>(_data.data() + loop.pos)));
  }

  LOG(TRACE) << "SPI (" << devicePath() << ") address " << to_hex_string(address) << "\n\t Wrote block data (Reg: data): \""
             << listVector(data, ", ", true) << "\"\n\t Read  block data (Reg: data): \"" << listVector(rx, ", ", true)
             << "\"";

  return rx;
}

std::vector<spi_t> iface_spi::read(const spi_address_t& address, const spi_reg_t reg, const unsigned int length) {
  std::vector<spi_t> rx(length);
  return write(address, reg, rx);
}
