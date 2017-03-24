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
#include "spi_CLICpix2.hpp"
#include "loopback.hpp"

using namespace caribou;

caribouHAL::caribouHAL(IFACE interface, std::string device_path, uint32_t device_address) :
  _iface(interface), _devpath(device_path), _devaddress(device_address) {

  // try to access the device:
  switch(_iface) {
  case IFACE::SPI : {
    iface_spi & myspi = interface_manager::getInterface<iface_spi>(_devpath);
    myspi.lock_address(device_address);
    break;
  }
  case IFACE::SPI_CLICpix2 : {
    iface_spi_CLICpix2 & myspi = interface_manager::getInterface<iface_spi_CLICpix2>(_devpath);
    myspi.lock_address(device_address);
    break;
  }
  case IFACE::I2C : {
    iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(_devpath);
    myi2c.lock_address(device_address);
    break;
  }
  case IFACE::LOOPBACK : {
    Interface<uint8_t, uint8_t> & loop = interface_manager::getInterface<iface_loopback>(_devpath);
    loop.lock_address(device_address);
    break;
  }
  default:
    throw caribou::CommunicationError("No device interface configured!");
  }
    
  //Disable all Voltage Regulators
  LOG(logDEBUGHAL) << "Disabling all Voltage regulators";
  iface_i2c & i2c0 = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  i2c0.write(ADDR_IOEXP, 0x2, {0x00, 0x00} ); //disable all bits of Port 1-2 (internal register)
  i2c0.write(ADDR_IOEXP, 0x6, {0x00, 0x00} ); //set all bits of Port 1-2 in output mode

  LOG(logDEBUGHAL) << "Disabling all current sources";
  powerDAC( false, std::get<1>(CUR_1), std::get<2>(CUR_1));
  powerDAC( false, std::get<1>(CUR_2), std::get<2>(CUR_2));
  powerDAC( false, std::get<1>(CUR_3), std::get<2>(CUR_3));
  powerDAC( false, std::get<1>(CUR_4), std::get<2>(CUR_4));
  powerDAC( false, std::get<1>(CUR_5), std::get<2>(CUR_5));
  powerDAC( false, std::get<1>(CUR_6), std::get<2>(CUR_6));
  powerDAC( false, std::get<1>(CUR_7), std::get<2>(CUR_7));
  powerDAC( false, std::get<1>(CUR_8), std::get<2>(CUR_8));
  
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

void caribouHAL::setVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const double voltage, const double maxExpectedCurrent){
  LOG(logDEBUGHAL) << "Setting " << voltage << "V "
		   << "on " << std::get<0>(regulator);

  if( voltage > 3.6 )
    throw ConfigInvalid( "Trying to set Voltage regulator to " + std::to_string(voltage) + " V (max is 3.6 V)");
  
  setDACVoltage(std::get<1>(regulator), std::get<2>(regulator), 3.6 - voltage );

  //set current/power monitor
  setCurrentMonitor(std::get<4>(regulator), maxExpectedCurrent );
}

void caribouHAL::powerVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const bool enable){

  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);

  if(enable){
    LOG(logDEBUGHAL) << "Powering up " << std::get<0>(regulator);

    //First power on DAC
    powerDAC(true, std::get<1>(regulator), std::get<2>(regulator));
    //Power on the Voltage regulator
    auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
    mask |=  1 <<  std::get<3>(regulator);
    i2c.write(ADDR_IOEXP, std::make_pair( 0x03, mask ));
  }
  else{
    LOG(logDEBUGHAL) << "Powering down " << std::get<0>(regulator);
    
    //Disable the Volage regulator
    auto mask = i2c.read(ADDR_IOEXP, 0x03, 1)[0];
    mask &= ~( 1 <<  std::get<3>(regulator));
    i2c.write(ADDR_IOEXP, std::make_pair( 0x03, mask ) );

    //Disable the DAC
    powerDAC( false, std::get<1>(regulator), std::get<2>(regulator) );
  }
}

void caribouHAL::setCurrentSource(const CURRENT_SOURCE_T source, const unsigned int current,
				  const CURRENT_SOURCE_POLARISATION_T polarisation){

  LOG(logDEBUGHAL) << "Setting " << current  << "uA "
		   << "on " << std::get<0>(source);

  if( current > 1000 )
    throw ConfigInvalid( "Tring to set current source to " + std::to_string(current) + " uA (max is 1000uA)");

  //set DAC
  setDACVoltage(std::get<1>(source), std::get<2>(source), (current * CAR_VREF_4P0)/1000 );

  //set polarisation
  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  auto mask = i2c.read(ADDR_IOEXP, 0x02, 1)[0];
  switch(polarisation){
  case PULL : mask &= ~( 1 <<  std::get<3>(source));
  case PUSH : mask |=    1 <<  std::get<3>(source);
  }
  i2c.write(ADDR_IOEXP, std::make_pair(0x02, mask) );
}

void caribouHAL::powerCurrentSource(const CURRENT_SOURCE_T source, const bool enable){
  if(enable){
    LOG(logDEBUGHAL) << "Powering up " << std::get<0>(source);
    powerDAC( true, std::get<1>(source), std::get<2>(source));
  }
  else{
    LOG(logDEBUGHAL) << "Powering down " << std::get<0>(source);
    powerDAC( true, std::get<1>(source), std::get<2>(source));
  }
}

void caribouHAL::setDACVoltage(const uint8_t device, const uint8_t address, const double voltage) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:  
  LOG(logDEBUGHAL) << "Setting voltage " << voltage << "V "
		   << "on DAC7678 at " << to_hex_string(device)
		   << " channel " << to_hex_string(address);
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C3);

  // Per default, the internal reference is switched off,
  // with external reference we have: voltage = d_in/4096*v_refin
  uint16_t d_in = voltage*4096/CAR_VREF_4P0;

  // with internal reference of 2.5V we have: voltage = d_in/4096*2*2.5
  //  -> d_in = voltage/2.5 * 4096/2

  //Check out of range values
  if (d_in >= 4096)
    d_in = 4095; 

  std::vector<uint8_t> command = {
    static_cast<uint8_t>( d_in >> 4 ),
    static_cast<uint8_t>( d_in << 4) };
  
  
  // Set DAC and update: combine command with channel via Control&Access byte:
  uint8_t reg = REG_DAC_WRUP_CHANNEL | address;
  
  // Send I2C write command
  myi2c.write(device, reg, command);
}

void caribouHAL::powerDAC(const bool enable, const uint8_t device, const uint8_t address) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:  
  LOG(logDEBUGHAL) << "Powering "
		   << (enable ? "up" : "down")
		   << " channel " << to_hex_string(address)
		   << " on DAC7678 at " << to_hex_string(device);
  iface_i2c & myi2c = interface_manager::getInterface<iface_i2c>(BUS_I2C3);

  // Set the correct channel bit to be powered up/down:
  uint16_t channel_bits = 2 << address;
  std::vector<uint8_t> command = {
    static_cast<uint8_t>( (enable ? (REG_DAC_POWERUP | channel_bits >> 4) : (REG_DAC_POWERDOWN_HZ | channel_bits >> 4)) & 0xFF ),
    static_cast<uint8_t>( (channel_bits << 4) & 0xFF )
  };
 
 // Send I2C write command
 myi2c.write(device, REG_DAC_POWER, command);
}

void caribouHAL::configureSI5345(SI5345_REG_T const * const regs,const size_t length){
  LOG(logDEBUGHAL) << "Configuring SI5345";

  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  uint8_t page = regs[0].address >> 8;  //first page to be used

  i2c.write(ADDR_CLKGEN, std::make_pair( 0x01, page ) );   //set first page

  for(size_t i = 0; i< length; i++){
    if(page != regs[i].address >> 8){  //adjust page
      page = regs[i].address >> 8;
      i2c.write(ADDR_CLKGEN, std::make_pair( 0x01, page) );
    }
    i2c.write(ADDR_CLKGEN, std::make_pair( regs[i].address & 0xFF, regs[i].value) );
  }
}

bool caribouHAL::isLockedSI5345(){
  LOG(logDEBUGHAL) << "Checking lock status of SI5345";

  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C0);
  i2c.write(ADDR_CLKGEN, std::make_pair( 0x01, 0x00 ) );   //set first page
  std::vector<i2c_t> rx = i2c.read(ADDR_CLKGEN, static_cast<uint8_t>( 0x0E) );
  if(rx[0] & 0x1){
    LOG(logDEBUGHAL) << "SI5345 is not locked";
    return false;
  }
  else{
    LOG(logDEBUGHAL) << "SI5345 is locked";
    return true;
  }
}


void caribouHAL::setCurrentMonitor(const uint8_t device, const double maxExpectedCurrent){
  LOG(logDEBUGHAL) << "Setting maxExpectedCurrent " << maxExpectedCurrent << "A "
		   << "on INA226 at " << to_hex_string(device);
  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C1);

  // Set configuration register:
  uint16_t conf = (1<<14);
  // Average over 16 samples:
  conf |= (1<<10);
  // Use a bus voltage conversion time of 8.244 ms:
  conf |= (0x7<<6);
  // Use a bus voltage conversion time of 8.244 ms:
  conf |= (0x7<<3);
  // Operation mode: continuous mesaurement of shunt and bus voltage:
  conf |= 0x7;
  i2c.write(device, REG_ADC_CONFIGURATION, {static_cast<i2c_t>(conf>>8), static_cast<i2c_t>(conf&0xFF)});

  // set calibration register
  
  // Calculate current LSB from expected current:
  double current_lsb = maxExpectedCurrent/(0x1<<15);
  LOG(logDEBUGHAL) << "  current_lsb  = " << static_cast<double>(current_lsb*1e6) << " uA/bit";
  
  unsigned int cal = static_cast<unsigned int>(0.00512/(CAR_INA226_R_SHUNT*current_lsb));
  LOG(logDEBUGHAL) << "  cal_register = " << static_cast<double>(cal)
		   << " (" << to_hex_string(cal) << ")";
  
  i2c.write(device, REG_ADC_CALIBRATION, {static_cast<i2c_t>(cal>>8), static_cast<i2c_t>(cal&0xFF)});
}

double caribouHAL::measureVoltage(const VOLTAGE_REGULATOR_T regulator){

  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C1);
  const i2c_address_t device = std::get<4>(regulator);

  LOG(logDEBUGHAL) <<  "Reading bus voltage from INA226 at " << to_hex_string(device);
  std::vector<i2c_t> voltage = i2c.read( device, REG_ADC_BUS_VOLTAGE, 2);

  // INA226: fixed LSB for voltage measurement: 1.25mV
  return ( static_cast<unsigned int>( voltage.at(0) << 8 ) | voltage.at(1) ) * 0.00125;
}

//FIXME: somtimes it returns dummy values
double caribouHAL::measureCurrent(const VOLTAGE_REGULATOR_T regulator){
  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C1);
  const i2c_address_t device = std::get<4>(regulator);
  LOG(logDEBUGHAL) <<  "Reading current from INA226 at " << to_hex_string(device);

  // Reading back the calibration register:
  std::vector<i2c_t> cal_v = i2c.read( device, REG_ADC_CALIBRATION, 2);
  double current_lsb = static_cast<double>(0.00512)/((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1))*CAR_INA226_R_SHUNT);
  LOG(logDEBUGHAL) << "  current_lsb  = " << static_cast<double>(current_lsb*1e6) << " uA/bit";

  // Reading the current register:
  std::vector<i2c_t> current_raw = i2c.read(device, REG_ADC_CURRENT, 2);
  return (static_cast<unsigned int>(current_raw.at(0) << 8 ) | current_raw.at(1) ) * current_lsb;
}

//FIXME: sometimes it return dummy values
double caribouHAL::measurePower(const VOLTAGE_REGULATOR_T regulator){
  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C1);
  const i2c_address_t device = std::get<4>(regulator);
  LOG(logDEBUGHAL) <<  "Reading power from INA226 at " << to_hex_string(device);

    // Reading back the calibration register:
  std::vector<i2c_t> cal_v = i2c.read( device, REG_ADC_CALIBRATION, 2);
  double power_lsb = static_cast<double>(0.00512)/((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1))*CAR_INA226_R_SHUNT);
  LOG(logDEBUGHAL) << "  power_lsb  = " << static_cast<double>(power_lsb*1e6) << " uA/bit";

  // Reading the power register:
  std::vector<i2c_t> power_raw = i2c.read(device, REG_ADC_POWER, 2);
  return (static_cast<unsigned int>(power_raw[0] << 8 ) | power_raw[1] ) * power_lsb;
}


double caribouHAL::readSlowADC(const uint8_t channel) {
  iface_i2c & i2c = interface_manager::getInterface<iface_i2c>(BUS_I2C3);

  LOG(logDEBUGHAL) << "Sampling channel " << static_cast<int>(channel)
		   << " on ADS7828 at " << to_hex_string(ADDR_ADC);

  unsigned char channels[8] = {0x00, 0x40, 0x10, 0x50, 0x20, 0x60, 0x30, 0x70};
  uint8_t command = channels[channel];

  // Enable single-ended mode, no differential measurement:
  command = command ^ 0x80;

  // We use the external reference voltage CAR_VREF_4P0, so do not switch to internal:
  //command = command ^ 0x08;

  std::vector<i2c_t> volt_raw = i2c.read(ADDR_ADC, command, 2);
  uint16_t readback = (volt_raw[0] << 8) | volt_raw[0];
  return static_cast<double>(readback*CAR_VREF_4P0/4096);
}
