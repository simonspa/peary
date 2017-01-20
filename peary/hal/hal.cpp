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

caribouHAL::caribouHAL(IFACE interface) :
  _iface(interface) {
  
  LOG(logDEBUGHAL) << "Configured device requires typ-" << (int)interface << " interface.";
}

caribouHAL::~caribouHAL() {}

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

std::vector<uint8_t> caribouHAL::sendCommand(uint8_t address, std::vector<uint8_t> data) {

  LOG(logDEBUGHAL) << "Prepare sending command to address " << static_cast<int>(address);

  // Send the command to the selected interface:
  switch(_iface) {
  case IFACE::SPI : {
    LOG(logDEBUGHAL) << "Command to SPI";
    caribou::iface_spi * cif = iface_spi::getInterface();
    return cif->sendCommand(address,data);
    break;
  }
  case IFACE::I2C : {
    LOG(logDEBUGHAL) << "Command to I2C";
    // caribou::iface_i2c * i2c = iface_i2c::getInterface();
    // i2c->sendCommand(address,data);
    return std::vector<uint8_t>();
    break;
  }
  default:
    throw caribou::CommunicationError("No device interface configured!");
  }
}