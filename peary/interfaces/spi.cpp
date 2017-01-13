/**
 * Caribou SPI interface class implementation
 */

#include "log.hpp"
#include "spi.hpp"

using namespace caribou;

std::vector<uint8_t> iface_spi::sendCommand(uint8_t address, std::vector<uint8_t> data) {

  // Cache the return values
  std::vector<uint8_t> miso_values;

  for(uint8_t d : data) {
    uint8_t retval = sendCommand(address, d);
      miso_values.push_back(retval);
  }

  return miso_values;
}

uint8_t iface_spi::sendCommand(uint8_t address, uint8_t data) {

  LOG(logINTERFACE) << "SPI: Sending data \"" << static_cast<int>(data)
		   << "\" to addr \"" <<  static_cast<int>(address) << "\"";

  std::lock_guard<std::mutex> lock(mutex);

  // FIXME: Implement sending of the MOSI command
  // FIXME: Implement retrieval of MISO value
  return 0;
}
