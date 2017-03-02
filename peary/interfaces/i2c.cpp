/**
 * Caribou I2C interface class implementation
 */

#include <cerrno>
#include <cstring>

//OS I2C support
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef YOCTO_COMPILATION
  #include <linux/i2c-dev-user.h>
#else
  #include <linux/i2c-dev.h>
#endif

#include "utils.hpp"
#include "log.hpp"

#include "i2c.hpp"

using namespace caribou;

iface_i2c::iface_i2c(std::string const & device_path) : Interface( device_path ) {
  if ( (i2cDesc = open( devicePath.c_str(), O_RDWR) ) < 0  ){
    throw DeviceException("Open " + device_path + " device failed. " + std::strerror(i2cDesc) );
  }
  LOG(logINTERFACE) << "Opened I2C device at " << device_path;
}

iface_i2c::~iface_i2c() {
  close(i2cDesc);
}


void iface_i2c::setAddress(i2c_address_t const address){
  if( ioctl(i2cDesc, I2C_SLAVE, address) < 0)
    throw CommunicationError( "Failed to acquire bus access and/or talk to slave (" + to_hex_string(address) + ") on " + devicePath + 
			      ": " + std::strerror(errno) );

  LOG(logINTERFACE) << "Talking to I2C slave at address " << to_hex_string(address);
}

i2c_t iface_i2c::write(const i2c_address_t& address, const i2c_t& data ){
  std::lock_guard<std::mutex> lock(mutex);
  
  setAddress(address);
  
  LOG(logINTERFACE) << "I2C (" << devicePath <<") address " << to_hex_string(address) << ": Writing data \"" << to_hex_string(data) <<  "\"";

  if( i2c_smbus_write_byte( i2cDesc, data) )
    throw CommunicationError( "Failed to write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  return 0;
}

std::vector<i2c_t> iface_i2c::write(const i2c_address_t&, const std::vector<i2c_t>&){
  throw CommunicationError( "Block write operation is not possible to a device without internal registers" );
  return std::vector<i2c_t>();
}

std::pair<i2c_reg_t, i2c_t> iface_i2c::write(const i2c_address_t& address,const std::pair<i2c_reg_t, i2c_t> & data){

  std::lock_guard<std::mutex> lock(mutex);
  
  setAddress(address);
  
  LOG(logINTERFACE) << "I2C (" << devicePath <<") address " << to_hex_string(address) << ": Register " << to_hex_string(data.first)
		    << " Writing data \"" << to_hex_string(data.second) <<  "\"";


  if( i2c_smbus_write_byte_data( i2cDesc, data.first, data.second ) )
    throw CommunicationError( "Failed to write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  return std::make_pair( 0, 0);
}
  
std::vector<i2c_t> iface_i2c::write(const i2c_address_t& address, const i2c_t & reg, const std::vector< i2c_t > & data){

  std::lock_guard<std::mutex> lock(mutex);
    
  setAddress(address);

  LOG(logINTERFACE) << "I2C (" << devicePath <<") address " << to_hex_string(address) << ": Register " << to_hex_string(reg)
		    << "\n\t Writing block data: \"" << listVector(data, ", ", true) << "\"";

  if( i2c_smbus_write_i2c_block_data( i2cDesc, reg, data.size(), data.data() ) )
    throw CommunicationError( "Failed to block write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  return std::vector<i2c_t>();
}

std::vector< std::pair<i2c_reg_t, i2c_t> > iface_i2c::write(const i2c_address_t&, const std::vector< std::pair<i2c_reg_t, i2c_t> >&){
  throw CommunicationError( "Block write operation with different variate register address is not supported by this I2C implementation" );
  return std::vector< std::pair<i2c_reg_t, i2c_t> >();
}

std::vector<i2c_t> iface_i2c::read(const i2c_address_t& address, const unsigned int length) {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;
  int temp;

  if(length != 1)
    throw CommunicationError( "Read operation of multiple data from the device without internal registers is not possible" );

  setAddress(address);
    
  temp = i2c_smbus_read_byte( i2cDesc );
  if( temp < 0 )
    throw CommunicationError( "Failed to read slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  data.push_back( temp );
  
  LOG(logINTERFACE) << "I2C (" << devicePath <<") address " << to_hex_string(address) << ": Read data \"" << to_hex_string(data[0]) <<  "\"";

  return data;
}

std::vector<i2c_t> iface_i2c::read(const i2c_address_t& address, const i2c_reg_t reg, const unsigned int length){

  std::lock_guard<std::mutex> lock(mutex);
  std::vector<i2c_t> data;

  setAddress(address);
    
  data.resize(length);
  if(i2c_smbus_read_i2c_block_data(i2cDesc, reg, length, data.data()) != length)
    throw CommunicationError( "Failed to read slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );

  LOG(logINTERFACE) << "I2C (" << devicePath <<") address " << to_hex_string(address) << ": Register " << to_hex_string(reg)
		    << "\n\t Read block data \"" << listVector( data, ", ", true) << "\"";

  return data;
}


std::vector<i2c_t> iface_i2c::wordwrite(const i2c_address_t& address, const uint16_t & reg, const std::vector< i2c_t > & data) {
  return std::vector<i2c_t>();
}

std::vector<i2c_t> iface_i2c::wordread(const i2c_address_t& address, const uint16_t reg, const unsigned int length) {
  std::vector<i2c_t>();
}
