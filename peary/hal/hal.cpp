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

//
//Initialize static members of the HAL
//

const std::map< VOLTAGE_REGULATOR_T, std::tuple<uint8_t,uint8_t, uint8_t, std::string> > caribouHAL:: voltageRegulatorMap = caribouHAL::create_map();

//
//Define functions of the HAL
//

caribouHAL::caribouHAL(IFACE interface, std::string device_path = "") :
  _iface(interface), _devpath(device_path) {

  //Disable all Voltage Regulators
  LOG(logDEBUGHAL) << "Disabling all Voltage regulators";
  iface_i2c & i2c0 = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  i2c0.write(ADDR_IOEXP, 0x2, {0x00, 0x00} ); //disable all bits of Port 1-2 (internal register)
  i2c0.write(ADDR_IOEXP, 0x6, {0x00, 0x00} ); //set all bits of Port 1-2 in output mode

  LOG(logDEBUGHAL) << "Configured device with typ-" << (int)_iface << " interface on " << _devpath;
}

caribouHAL::~caribouHAL() {}

uint32_t caribouHAL::getFirmwareRegister(uint16_t) {
  throw FirmwareException("Functionality not implemented.");
}

std::map< VOLTAGE_REGULATOR_T, std::tuple<uint8_t,uint8_t, uint8_t, std::string> > caribouHAL::create_map(){
  std::map< VOLTAGE_REGULATOR_T, std::tuple<uint8_t,uint8_t, uint8_t, std::string> > m;
  m[ PWR_OUT1 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTA, 7, ADDR_MONITOR_U53, "PWR_OUT1" );
  m[ PWR_OUT2 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTC, 6, ADDR_MONITOR_U52, "PWR_OUT2" );
  m[ PWR_OUT3 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTE, 5, ADDR_MONITOR_U55, "PWR_OUT3" );
  m[ PWR_OUT4 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTG, 4, ADDR_MONITOR_U54, "PWR_OUT4" );
  m[ PWR_OUT5 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTB, 0, ADDR_MONITOR_U57, "PWR_OUT5" );
  m[ PWR_OUT6 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTD, 1, ADDR_MONITOR_U56, "PWR_OUT6" );
  m[ PWR_OUT7 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTF, 2, ADDR_MONITOR_U59, "PWR_OUT7" );
  m[ PWR_OUT8 ] = std::make_tuple( REG_DAC_CHANNEL_VOUTH, 3, ADDR_MONITOR_U58, "PWR_OUT8" );
  return m;
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

void caribouHAL::setVoltageRegulator(VOLTAGE_REGULATOR_T regulator, double voltage){
  LOG(logDEBUGHAL) << "Setting " << voltage << "V "
		   << "on " << std::get<3>(  voltageRegulatorMap.at( regulator ) );

  if( voltage > 3.6 )
    throw ConfigInvalid( "Tring to set Voltage regulator to " + std::to_string(voltage) + " V (max is 3.6 V)");
  
  setDACVoltage( ADDR_DAC_U50,  std::get<0>( voltageRegulatorMap.at( regulator ) ), 3.6 - voltage );
}

void caribouHAL::powerVoltageRegulator(VOLTAGE_REGULATOR_T regulator, bool enable){

  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  
  if(enable){
    LOG(logDEBUGHAL) << "Powering up " << std::get<3>(  voltageRegulatorMap.at( regulator ) );

    //First power on DAC
    powerDAC( true, ADDR_DAC_U50, std::get<0>( voltageRegulatorMap.at( regulator ) ) );
    //Power on the Voltage regulator
    auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
    mask |=  1 <<  std::get<1>( voltageRegulatorMap.at( regulator ) );
    i2c.write(ADDR_IOEXP, std::make_pair( 0x03, mask ));
  }
  else{
    LOG(logDEBUGHAL) << "Powering down " << std::get<3>(  voltageRegulatorMap.at( regulator ) );
    
    //Disable the Volage regulator
    auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
    mask &= ~( 1 <<  std::get<1>( voltageRegulatorMap.at( regulator ) ) );
    i2c.write(ADDR_IOEXP, std::make_pair( 0x03, mask ) );

    //Disable the DAC
    powerDAC( false, ADDR_DAC_U50, std::get<0>( voltageRegulatorMap.at( regulator ) ) );
  }
}

void caribouHAL::setDACVoltage(uint8_t device, uint8_t address, double voltage) {

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

void caribouHAL::powerDAC(bool enable, uint8_t device, uint8_t address) {

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
