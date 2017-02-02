#ifndef CARIBOU_HAL_INTERFACE_HPP
#define CARIBOU_HAL_INTERFACE_HPP

#include <vector>
#include <cstdint>
#include <utility>

#include "exceptions.hpp"

//Abstract class for all interfaces
//@param ADDRESS_T : type for a device address
//@param REG_T : type for register addresses
//@param DATA_T : type for data 
template <typename ADDRESS_T = uint8_t, typename DATA_T = uint8_t, typename REG_T = DATA_T >
virtual class Interface {

public: 
  //////////////////////
  // Write operations
  //////////////////////

  //Write data to a device which does not contain internal register
  //If readout is intergralpart of write operations, the read values a returned by function. 
  virtual std::vector<DATA_T>& write(const ADDRESS_T& address,const DATA_T& data){
    std::vector<DATA_T> data_v {data};
    return write(address, data_v);
  }
  
  //Write data to a device which does not contain internal register
  //If readout is intergralpart of write operations, the read values a returned by function. 
  std::vector<DATA_T>& write(const ADDRESS_T& address, const std::vector<DATA_T>& data) = 0;

  //Write data to a device containing internal registers
  //If readout is intergralpart of write operations, the read values a returned by function. 
  virtual std::vector<DATA_T> & write(const ADDRESS_T& address,const std::pair<REG_T, DATA_T> & data){
    std::vector<std::pair<REG_T, DATA_T> > data_v {data};
    return write(address, data_v);
  }
  
  //Write data to a device containing internal registers
  //If readout is intergralpart of write operations, the read values a returned by function. 
  std::vector<DATA_T>& write(const ADDRESS_T& address,const std::vector<std::pair<REG_T, DATA_T> >& data) = 0;

  //////////////////////
  // Read operations
  //////////////////////
  
  //Read number of data words form the given device
  std::vector<DATA_T>& read(const ADDRESS_T& address, const unsigned int& length) = 0;

  //Read number of data words form a register of the given device
  std::vector<DATA_T>& read(const ADDRESS_T& address, const REG_T reg, const unsigned int& length) = 0;

}

#endif
