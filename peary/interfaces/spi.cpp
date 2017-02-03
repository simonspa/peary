/**
 * Caribou SPI interface class implementation
 */

#include "log.hpp"
#include "spi.hpp"

using namespace caribou;

std::vector<spi_t> iface_spi::write(const spi_t& address, const spi_t& data) {
  return std::vector<spi_t>();
}

std::vector<spi_t> iface_spi::write(const spi_t& address, const std::vector<spi_t>& data) {

  // Cache the return values
  std::vector<spi_t> miso_values;

  for(auto d : data) {
    auto retval = write(address, d);
    miso_values.insert(std::end(miso_values), std::begin(retval), std::end(retval));
  }

  return miso_values;
}

std::vector<spi_t> iface_spi::write(const spi_t& address,const std::pair<spi_t, spi_t> & data) {}
std::vector<spi_t> iface_spi::write(const spi_t& address, const spi_t & reg, const std::vector< spi_t > & data) {}
std::vector<spi_t> iface_spi::write(const spi_t& address,const std::vector< std::pair<spi_t, spi_t> > & data) {}

std::vector<spi_t> iface_spi::read(const spi_t& address, const unsigned int& length) {
  throw NoDataAvailable("");
}

std::vector<spi_t> iface_spi::read(const spi_t& address, const spi_t reg, const unsigned int& length) {
  throw NoDataAvailable("");
}
