/**
 * Caribou SPI interface class implementation
 */

#include <climits>
#include <cstring>
#include <utility>

#include "utils.hpp"
#include "log.hpp"
#include "spi_CLICpix2.hpp"


using namespace caribou;

std::pair<spi_reg_t, spi_t> iface_spi_CLICpix2::write(const spi_address_t& address, const std::pair<spi_reg_t, spi_t> & data){

  std::lock_guard<std::mutex> lock(mutex);
  std::array< uint8_t, sizeof(spi_reg_t) + sizeof(spi_t) >  _data;
  
  std::memcpy( _data.data(), &data.second, sizeof(spi_t) );
  std::memcpy( _data.data() + sizeof(spi_t), &data.first, sizeof( spi_reg_t) );
    
  std::pair<spi_reg_t, spi_t> rx (* static_cast<spi_reg_t* >( _data.data() + sizeof(spi_t) ), * static_cast<spi_t* >( _data.data()  ) );

  LOG(logINTERFACE) << "SPI (" << devicePath <<") address " << to_hex_string(address) << ": Register " << to_hex_string(data.first)
		    << " Wrote data \"" << to_hex_string(data.second) << "\" Read data \"" << to_hex_string(rx.second) << "\"";

  return rx;
}

std::vector< std::pair<spi_reg_t, spi_t> > iface_spi_CLICpix2::write(const spi_address_t& address, const std::vector< std::pair<spi_reg_t, spi_t> > & data){

  std::lock_guard<std::mutex> lock(mutex);

  std::vector< uint8_t > _data( ( sizeof(spi_reg_t) + sizeof(spi_t) ) * data.size() );
  std::vector< std::pair<spi_reg_t, spi_t> > rx;

  //pack
  for( struct { unsigned int i = 0; unsigned int pos = 0;} loop; loop.i < data.size(); ++loop.i) {
    std::memcpy( _data.data() + loop.pos , & data[loop.i].second, sizeof(spi_t) );
    loop.pos += sizeof(spi_t);
    std::memcpy( _data.data() + loop.pos + sizeof(spi_t),   & data[loop.i].first,  sizeof(spi_reg_t) );
    loop.pos += sizeof(spi_reg_t);
  }

  //unpack
  rx.reserve( data.size() );
  for( struct { unsigned int i = 0; unsigned int pos = 0;} loop; loop.i < data.size(); ++loop.i) {
    rx.push_back( std::make_pair( * static_cast<spi_reg_t*>( _data.data() + loop.pos + sizeof(spi_t) ),
				  * static_cast<spi_t*>( _data.data() + loop.pos ) ));
  }
  
  LOG(logINTERFACE) << "SPI (" << devicePath <<") address " << to_hex_string(address) << "\n\t Wrote block data (Reg: data): \""
		    << listVector( data, ", ", true) << "\"\n\t Read  block data (Reg: data): \"" <<  listVector( rx, ", ", true) << "\"";
  
  return rx;
}



