/**
 * Caribou SPI interface class implementation
 */

#include <climits>
#include <cstring>
#include <utility>

//OS SPI support
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "utils.hpp"
#include "log.hpp"
#include "spi.hpp"


using namespace caribou;

iface_spi::iface_spi(std::string const & device_path) : Interface( device_path ) {
  std::lock_guard<std::mutex> lock(mutex);

  //Open device
  if ( (spiDesc = open( devicePath.c_str(), O_RDWR) ) < 0  ){
    throw DeviceException("Open " + device_path + " device failed. " + std::strerror(spiDesc) );
  }

  if( ioctl( spiDesc, SPI_IOC_WR_MODE32 , &mode) == -1 ){
    throw DeviceException("Set mode for " + device_path + " device failed. " + std::strerror(spiDesc) );
  }
  if( ioctl( spiDesc,  SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1 ){
    throw DeviceException("Set speed for " + device_path + " device failed. " + std::strerror(spiDesc) );
  }

  
}

iface_spi::~iface_spi() {
  close(spiDesc);
}


spi_t iface_spi::write(const spi_t& address, const spi_t& data) {
  std::lock_guard<std::mutex> lock(mutex);
  spi_t rx;
  
  spi_ioc_transfer tr = spi_ioc_transfer();
  tr.tx_buf = (unsigned long)  & data;
  tr.rx_buf = (unsigned long) &rx;
  tr.len = sizeof(spi_t);
  tr.bits_per_word = sizeof(spi_t) * CHAR_BIT;

  if( ioctl(spiDesc, SPI_IOC_MESSAGE(1), &tr) < 1 ){
    throw CommunicationError( "Failed to access (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  }

  LOG(logINTERFACE) << std::hex << "SPI (" << devicePath <<") address " << static_cast<unsigned int>(address) << ": Wrote data \"" << static_cast<unsigned int>(data)
		    <<  "\" Read data \"" << static_cast<unsigned int>(rx) <<  "\"" << std::dec << std::endl;

  return rx;
}

std::vector<spi_t> iface_spi::write(const spi_address_t& address, const std::vector<spi_t>& data ){
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<spi_t> rx;
  rx.resize( data.size() );

  std::unique_ptr<  spi_ioc_transfer []> tr( new  spi_ioc_transfer [ data.size() ]() );

  for( auto i = 0; i< data.size(); ++i) {
    tr[i].tx_buf = (unsigned long) &data[i];
    tr[i].rx_buf = (unsigned long) &rx[i];
    tr[i].len = sizeof(spi_t);
    tr[i].bits_per_word = sizeof(spi_t) * CHAR_BIT;
  }

  if( ioctl(spiDesc, SPI_IOC_MESSAGE(data.size()), tr.get() ) < data.size() ){
      throw CommunicationError( "Failed to access (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  }

  LOG(logINTERFACE) << std::hex << "SPI (" << devicePath <<") address " << static_cast<unsigned int>(address) << ": Wrote data \"";
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<unsigned int>(i) << " ";
  LOG(logINTERFACE) <<  "\"\n\t Read data \"";
  for( auto i : rx)
    LOG(logINTERFACE) << static_cast<unsigned int>(i) << " ";
  LOG(logINTERFACE) << "\"" << std::dec << std::endl;

  return rx;
}

std::pair<spi_reg_t, spi_t> iface_spi::write(const spi_address_t& address, const std::pair<spi_reg_t, spi_t> & data){

  std::lock_guard<std::mutex> lock(mutex);
  std::array< uint8_t, sizeof(spi_reg_t) + sizeof(spi_t) >  _data;
  
  std::memcpy( _data.data(), &data.second, sizeof(spi_t) );
  std::memcpy( _data.data() + sizeof(spi_t), &data.first, sizeof( spi_reg_t) );
    
  spi_ioc_transfer tr = spi_ioc_transfer();
  tr.tx_buf = (unsigned long) _data.data();
  tr.rx_buf = (unsigned long) _data.data();
  tr.len = sizeof(spi_reg_t) + sizeof(spi_t);
  tr.bits_per_word = ( sizeof(spi_reg_t) + sizeof(spi_t) ) * CHAR_BIT;
  
  if( ioctl(spiDesc, SPI_IOC_MESSAGE(1), &tr) < 1 ){
    throw CommunicationError( "Failed to access (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  }

  std::pair<spi_reg_t, spi_t> rx (* static_cast<spi_reg_t* >( _data.data() + sizeof(spi_t) ), * static_cast<spi_t* >( _data.data()  ) );

  LOG(logINTERFACE) << std::hex << "SPI (" << devicePath <<") address " << static_cast<int>(address) << ": Register " << static_cast<int>(data.first)
		    << " Wrote data \"" << static_cast<int>(data.second) << "\" Read data \"" << static_cast<unsigned int>(rx.second) << std::dec << std::endl;

  return rx;
}

std::vector<spi_t> iface_spi::write(const spi_address_t& address, const spi_t & reg, const std::vector< spi_t > & data){

  std::vector< std::pair<spi_reg_t, spi_t> > _data;
  std::vector<spi_t>  rx;

  _data.reserve( data.size() );
  for( auto i : data )
    _data.push_back( std::make_pair( reg, i) );

  _data = write( address, _data);

  rx.reserve( data.size() );
  for( auto i : _data)
    rx.push_back( i.second );

  return rx;
}

std::vector< std::pair<spi_reg_t, spi_t> > iface_spi::write(const spi_address_t& address, const std::vector< std::pair<spi_reg_t, spi_t> > & data){

  std::lock_guard<std::mutex> lock(mutex);

  std::vector< uint8_t > _data( ( sizeof(spi_reg_t) + sizeof(spi_t) ) * data.size() );
  std::unique_ptr<  spi_ioc_transfer []> tr( new  spi_ioc_transfer[data.size()]() );
  std::vector< std::pair<spi_reg_t, spi_t> > rx;

  //pack
  for( struct { unsigned int i = 0; unsigned int pos = 0;} loop; loop.i < data.size(); ++loop.i) {
    std::memcpy( _data.data() + loop.pos , & data[loop.i].second, sizeof(spi_t) );
    loop.pos += sizeof(spi_t);
    std::memcpy( _data.data() + loop.pos + sizeof(spi_t),   & data[loop.i].first,  sizeof(spi_reg_t) );
    loop.pos += sizeof(spi_reg_t);

    tr[loop.i].tx_buf =  (unsigned long) _data.data() + ( sizeof(spi_reg_t) + sizeof(spi_t) ) * loop.i;
    tr[loop.i].rx_buf =  (unsigned long) _data.data() + ( sizeof(spi_reg_t) + sizeof(spi_t) ) * loop.i;
    tr[loop.i].len = sizeof(spi_reg_t) + sizeof(spi_t);
    tr[loop.i].bits_per_word = ( sizeof(spi_reg_t) + sizeof(spi_t) ) * CHAR_BIT;
  }

  if( ioctl(spiDesc, SPI_IOC_MESSAGE(data.size()), tr.get() ) < data.size() ){
    throw CommunicationError( "Failed to access (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  }

  //unpack
  rx.reserve( data.size() );
  for( struct { unsigned int i = 0; unsigned int pos = 0;} loop; loop.i < data.size(); ++loop.i) {
    rx.push_back( std::make_pair( * static_cast<spi_reg_t*>( _data.data() + loop.pos + sizeof(spi_t) ),
				  * static_cast<spi_t*>( _data.data() + loop.pos ) ));
  }
  
  LOG(logINTERFACE) << std::hex << "SPI (" << devicePath <<") address " << static_cast<int>(address) << " Wrote block data (Reg: data): " << std::endl;
  for( auto i : data)
    LOG(logINTERFACE) << static_cast<unsigned int>( i.first ) << ": \"" << static_cast<unsigned int>(i.second) << "\" ";
  LOG(logINTERFACE) << " Read block data (Reg: data):" << std::endl;
  for( auto i : rx)
    LOG(logINTERFACE) << static_cast<unsigned int>( i.first ) << ": \"" << static_cast<unsigned int>(i.second) << "\" ";
  LOG(logINTERFACE) <<  std::dec << std::endl;
  
  return rx;
}

std::vector<spi_t> iface_spi::read(const spi_address_t& address, const unsigned int length){

  std::lock_guard<std::mutex> lock(mutex);

  std::vector<spi_t> rx(length);
  std::unique_ptr<  spi_ioc_transfer []> tr( new  spi_ioc_transfer[length]() );

  for( unsigned int i = 0; i < length; ++i) {
    tr[i].rx_buf = (unsigned long) rx.data()+i;
    tr[i].len = sizeof(spi_t);
    tr[i].bits_per_word = sizeof(spi_t)  * CHAR_BIT;
  }

  if( ioctl(spiDesc, SPI_IOC_MESSAGE(length), tr.get() ) < length ){
    throw CommunicationError( "Failed to access (" + to_hex_string(address) + ") on " + devicePath + ": " + std::strerror(errno) );
  }

  LOG(logINTERFACE) << std::hex << "SPI (" << devicePath <<") address " << static_cast<int>(address) << " Read block data: \"" << std::endl;
  for( auto i : rx)
    LOG(logINTERFACE) << static_cast<unsigned int>( i ) << " ";
  LOG(logINTERFACE) << "\"" <<  std::dec << std::endl;

  return rx;
}

std::vector<spi_t> iface_spi::read(const spi_address_t& address, const spi_reg_t reg, const unsigned int length){
  std::vector<spi_t> rx(length);
  return write(address, reg, rx);
}


