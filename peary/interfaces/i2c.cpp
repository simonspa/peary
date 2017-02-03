/**
 * Caribou I2C interface class implementation
 */

#include <cerrno>
#include <cstring>

//OS I2C support
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "utils.hpp"
#include "log.hpp"


#include <i2c.hpp>

using namespace caribou;

iface_i2c::iface_i2c(std::string const & device_path) : Interface( device_path ) {
  if ( (i2cDesc = open( devicePath.c_str(), O_RDWR) ) < 0  ){
    throw DeviceException("Open " + device_path + " device failed. " + std::strerror(i2cDesc) );
  }
}

void iface_i2c::setAddress(i2c_address_t const address){
  if( ioctl(i2cDesc, I2C_SLAVE, address) < 0)
    throw CommunicationError( "Failed to acquire bus access and/or talk to slave (" + to_hex_string(address) + ") on " + devicePath + 
			      ": " + std::strerror(errno) );
}

std::vector<i2c_t> iface_i2c::write(const i2c_t& address, const i2c_t& data ){
  std::lock_guard<std::mutex> lock(mutex);
  
  setAddress(address);
  
  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") address " << static_cast<int>(address) << ": Writing data \"" << static_cast<int>(data)
		    <<  "\"" << std::dec << std::endl;

  if( i2c_smbus_write_byte( i2cDesc, data) )
    throw CommunicationError( "Failed to write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
}

std::vector<i2c_t> iface_i2c::write(const i2c_t& address, const std::vector<i2c_t>& data ){
  throw CommunicationError( "Block write operation is not possible to a device without internal registers" );
}

std::vector<i2c_t> iface_i2c::write(const i2c_t& address,const std::pair<i2c_t, i2c_t> & data){

  std::lock_guard<std::mutex> lock(mutex);
  
  setAddress(address);
  
  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") address " << static_cast<int>(address) << ": Register \"" << static_cast<int>(data.first)
		    << "\" Writing data \"" << static_cast<int>(data.second) <<  "\"" << std::dec << std::endl;


  if( i2c_smbus_write_byte_data( i2cDesc, data.first, data.second ) )
    throw CommunicationError( "Failed to write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
}
  
std::vector<i2c_t> iface_i2c::write(const i2c_t& address, const i2c_t & reg, const std::vector< i2c_t > & data){

  std::lock_guard<std::mutex> lock(mutex);
    
  setAddress(address);

  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") address " << static_cast<int>(address) << ": Register \"" << static_cast<int>(reg)
		    << "\" Writing block data: \"";
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<int>(i) << " ";
  LOG(logINTERFACE) << "\""<< std::dec << std::endl;

  if( i2c_smbus_write_block_data( i2cDesc, reg, data.size(), data.data() ) )
    throw CommunicationError( "Failed to block write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
}

std::vector<i2c_t> iface_i2c::write(const i2c_t& address,const std::vector< std::pair<i2c_t, i2c_t> > & data){
  throw CommunicationError( "Block write operation with different variate register address is not supported by ths I2C implementation" );
}

std::vector<i2c_t> iface_i2c::read(const i2c_t& address, const unsigned int& length) {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;

  if(length != 1)
    throw CommunicationError( "Read operation of multiple data from the device without internal registers is not possible" );

  setAddress(address);
    
  data.resize(1);

  if( i2c_smbus_read_byte( i2cDesc ) )
    throw CommunicationError( "Failed to read slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") address " << static_cast<int>(address) << ": Read data \"" << static_cast<int>(data[0])
		    <<  "\"" << std::dec << std::endl;
  
}

std::vector<i2c_t> iface_i2c::read(const i2c_t& address, const i2c_t reg, const unsigned int& length){

  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;

  setAddress(address);
    
  data.resize(length);

  if( i2c_smbus_read_block_data( i2cDesc, reg, length, data.data() ) )
    throw CommunicationError( "Failed to read slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") address " << static_cast<int>(address) << ": Register \"" << static_cast<int>(reg)
		    << "\" Read block data \"";
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<int>(i) << " ";
  LOG(logINTERFACE) << "\""<< std::dec << std::endl;

}
