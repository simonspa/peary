/**
 * Caribou HAL class implementation
 */

#include "exceptions.h"
#include "constants.h"
#include "log.h"
#include "hal.h"

#include "spi.h"
#include "i2c.h"

using namespace caribou;

caribouHAL::caribouHAL(uint8_t interface) :
  _iface(IFACE::NONE) {
  
  LOG(logDEBUGHAL) << "Configured device requires typ-" << (int)interface << " interface.";
};

caribouHAL::~caribouHAL() {};

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

std::vector<uint8_t> caribouHAL::sendCommand(uint8_t address, std::vector<uint8_t> data) {

  // Send the command to the selected interface:
  switch(_iface) {
  case IFACE::SPI : {
    caribou::iface_spi * cif = iface_spi::getInterface();
    return cif->sendCommand(address,data);
    break;
  }
  case IFACE::I2C : {
    caribou::iface_i2c * i2c = iface_i2c::getInterface();
    i2c->sendCommand(address,data);
    return std::vector<uint8_t>();
    break;
  }
  default:
    throw caribou::CommunicationError("No device interface configured!");
  }
}
