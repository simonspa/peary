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

iface_i2c::iface_i2c(std::string const & device_path) : devicePath(device_path) {
  if ( (i2cDesc = open( devicePath.c_str(), O_RDWR) ) < 0  ){
    throw DeviceException("Open " + device_path + " device failed. " + std::strerror(i2cDesc) );
  }
}

void iface_i2c::setAddress(i2c_address_t const address){
  if( ioctl(i2cDesc, I2C_SLAVE, address) < 0)
    throw CommunicationError( "Failed to acquire bus access and/or talk to slave (" + to_hex_string(address) + ") on " + devicePath + 
			      ": " + std::strerror(errno) );
}

void iface_i2c::write(i2c_address_t const address, std::vector<i2c_data_t> & data) {

  std::lock_guard<std::mutex> lock(mutex);

  setAddress(address);
  
  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") : Writing data \"";
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<int>(i) << " ";
  LOG(logINTERFACE) << "\" to addr \"" <<  static_cast<int>(address) << "\"" << std::dec << std::endl;

  if( ::write( i2cDesc, data.data(), data.size() ) != static_cast<ssize_t>( data.size() ) )
    throw CommunicationError( "Failed to write slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
}

void iface_i2c::read(i2c_address_t const address, std::vector<i2c_data_t> & data,  const unsigned int length) {

  std::lock_guard<std::mutex> lock(mutex);

  setAddress(address);

  data.resize(length);
  
  if( ::read( i2cDesc, data.data(), data.size() ) != static_cast<ssize_t>( data.size() ) )
    throw CommunicationError( "Failed to read slave (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  

  LOG(logINTERFACE) << std::hex << "I2C (" << devicePath <<") : Read data \"";
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<int>(i) << " ";
  LOG(logINTERFACE) << "\" to addr \"" <<  static_cast<int>(address) << "\"" << std::dec << std::endl;

}



  virtual DATA_T& write(const ADDRESS_T& address,const DATA_T& data){
    std::vector<DATA_T> data_v {data};
    std::vector<DATA_T> ret = write(address, data_v);
    switch( ret_size() ) {
    case 0: return static_cast<DATA_T>( 0 );
    case 1: return ret[0];
    defualt:
      throw  DataException( "For single access read more than 1 
	}
