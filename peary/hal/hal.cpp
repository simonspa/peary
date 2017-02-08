/**
 * Caribou HAL class implementation
 */

#include "exceptions.hpp"
#include "constants.hpp"
#include "log.hpp"
#include "hal.hpp"

#include "interface_manager.hpp"
#include "interface.hpp"

#include "i2c.hpp"
#include "spi.hpp"
#include "loopback.hpp"

using namespace caribou;

caribouHAL::caribouHAL(IFACE interface, std::string device_path = "") :
  _iface(interface), _devpath(device_path) {
  
  LOG(logDEBUGHAL) << "Configured device with typ-" << (int)_iface << " interface on " << _devpath;
}

caribouHAL::~caribouHAL() {}

uint8_t caribouHAL::getDeviceID() {

  // FIXME: Implement reading of device identifier register
  return 0;
}

std::vector<uint8_t>& caribouHAL::write(uint8_t address, std::vector<uint8_t> data) {

  switch(_iface) {
  case IFACE::SPI : {
    LOG(logDEBUGHAL) << "Command to SPI";
    iface_spi & myspi = interface_manager::getInterface<iface_spi>(_devpath);
    myspi.write(address,data);
    break;
  }
  case IFACE::I2C : {
    LOG(logDEBUGHAL) << "Command to I2C";
    iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(_devpath);
    myi2c.write(address,data);
    break;
  }
  case IFACE::LOOPBACK : {
    LOG(logDEBUGHAL) << "Command to LOOPBACK interface";
    Interface<uint8_t, uint8_t> & loop = interface_manager::getInterface<iface_loopback>(_devpath);
    loop.write(0x0,address,data);
    break;
  }

  default:
    throw caribou::CommunicationError("No device interface configured!");
  }
}
