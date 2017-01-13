/**
 * Caribou I2C interface class implementation
 */

#include <i2c.hpp>

using namespace caribou;

void iface_i2c::sendCommand(i2c_address_t address, std::vector<i2c_data_t> data) {

  //  LOG(logINTERFACE) << std::hex << "I2C: Sending data \"" << static_cast<int>(data)
  //		    << "\" to addr \"" <<  static_cast<int>(address) << "\"" << std::dec;

  //std::lock_guard<std::mutex> lock(mutex);

  // FIXME: Implement sending of command
}
