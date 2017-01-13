/**
 * Caribou I2C interface class implementation
 */

#include "log.h"
#include "i2c.h"

using namespace caribou;

void iface_i2c::sendCommand(uint8_t address, std::vector<uint8_t> data) {

  for(uint8_t d : data) {
    sendCommand(address, d);
  }
}

void iface_i2c::sendCommand(uint8_t address, uint8_t data) {

  LOG(logINTERFACE) << "I2C: Sending data \"" << static_cast<int>(data)
		   << "\" to addr \"" <<  static_cast<int>(address) << "\"";

  std::lock_guard<std::mutex> lock(mutex);

  // FIXME: Implement sending of command
}
