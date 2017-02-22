/**
 * Caribou HAL class implementation
 */

#include "exceptions.hpp"
#include "constants.hpp"
#include "carboard.hpp"
#include "utils.hpp"
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

uint32_t caribouHAL::getFirmwareRegister(uint16_t) {
  throw FirmwareException("Functionality not implemented.");
}

uint8_t caribouHAL::getCaRBoardID() {

  LOG(logDEBUGHAL) << "Reading board ID from CaR EEPROM";
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);

  // Read one word from memory address on the EEPROM:
  // FIXME register address not set!
  std::vector<uint8_t> data =  myi2c.wordread(ADDR_EEPROM,0x0,1);

  if(data.empty()) throw CommunicationError("No data returned");
  return data.front();
}

std::vector<uint8_t> caribouHAL::write(const uint8_t address, const std::vector<uint8_t>& data) {

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

void caribouHAL::setVoltage(uint8_t device, uint8_t address, double voltage) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:  
  LOG(logDEBUGHAL) << "Setting voltage " << voltage << "V "
		   << "on DAC7678 at " << to_hex_string(device)
		   << " channel " << to_hex_string(address);
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C3);

  // Per default, the internal reference is switched off,
  // with external reference we have: voltage = d_in/4096*v_refin
  uint16_t d_in = voltage/CAR_VREF_4P0*4096;

  // with internal reference of 2.5V we have: voltage = d_in/4096*2*2.5
  //  -> d_in = voltage/2.5 * 4096/2
  
  std::vector<uint8_t> payload = {
    static_cast<uint8_t>((d_in >> 8)&0xFF),
    static_cast<uint8_t>(d_in&0xFF)
  };

  // Set DAC and update: combine command with channel via Control&Access byte:
  uint8_t reg = REG_DAC_WRUP_CHANNEL | address;
  
  // Send I2C write command
  myi2c.write(device,reg,payload);
}

void caribouHAL::powerVoltage(bool enable, uint8_t device, uint8_t address) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:  
  LOG(logDEBUGHAL) << "Powering "
		   << (enable ? "up" : "down")
		   << " channel " << to_hex_string(address)
		   << " on DAC7678 at " << to_hex_string(device);
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C3);

  // Set the correct channel bit to be powered down:
  uint16_t d_in = (1 << (address+5));
  std::vector<uint8_t> payload = {
    static_cast<uint8_t>((enable ? REG_DAC_POWERUP : REG_DAC_POWERDOWN_HZ) | ((d_in >> 8)&0xFF)),
    static_cast<uint8_t>(d_in&0xFF)
  };
  
  // Send I2C write command
  myi2c.write(device,REG_DAC_POWER,payload);
}
