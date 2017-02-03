/**
 * Caribou HAL class implementation
 */

#include "exceptions.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "hal.hpp"

#include "spi.hpp"
#include "i2c.hpp"

using namespace caribou;

caribouHAL::caribouHAL(IFACE interface, std::string device_path = "") :
  _iface(interface) {
  
  LOG(logDEBUGHAL) << "Configured device requires typ-" << (int)interface << " interface.";
}

caribouHAL::~caribouHAL() {}

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

