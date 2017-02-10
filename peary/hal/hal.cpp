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

  LOG(logDEBUGHAL) << "Reading device ID from EEPROM";
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);

  // Read one word from memory address on the EEPROM:
  // FIXME register address not set!
  std::vector<uint8_t> data =  myi2c.wordread(ADDR_EEPROM,0x0,1);

  if(data.empty()) throw CommunicationError("No data returned");
  return data.front();
}

std::vector<uint8_t> caribouHAL::write(uint8_t address, std::vector<uint8_t> data) {

  switch(_iface) {
  case IFACE::SPI : {
    LOG(logDEBUGHAL) << "Command to SPI";
    iface_spi & myspi = interface_manager::getInterface<iface_spi>(_devpath);
    return myspi.write(address,data);
    break;
  }
  case IFACE::I2C : {
    LOG(logDEBUGHAL) << "Command to I2C";
    iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(_devpath);
    return myi2c.write(address,data);
    break;
  }
  case IFACE::LOOPBACK : {
    LOG(logDEBUGHAL) << "Command to LOOPBACK interface";
    Interface<uint8_t, uint8_t> & loop = interface_manager::getInterface<iface_loopback>(_devpath);
    return loop.write(0x0,address,data);
    break;
  }

  default:
    throw caribou::CommunicationError("No device interface configured!");
  }
}

double caribouHAL::readTemperature() {

  // Two bytes must be read, containing 12bit of temperature information plus 4bit 0.
  // Negative numbers are represented in binary twos complement format.
  
  LOG(logDEBUGHAL) << "Reading temperature from TMP101";
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);

  // Read the two temperature bytes from the TMP101:
  std::vector<uint8_t> data =  myi2c.read(ADDR_TEMP,REG_TEMP_TEMP,2);
  if(data.size() != 2) throw CommunicationError("No data returned");

  // FIXME correctly handle 2's complement for negative temperatures!
  int16_t temp = ((data.front() << 8) | data.back()) >> 4;
  return temp*0.0625;
}
