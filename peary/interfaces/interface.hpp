#ifndef CARIBOU_HAL_INTERFACE_HPP
#define CARIBOU_HAL_INTERFACE_HPP

#include <vector>
#include <cstdint>
#include <utility>
#include <string>

#include "exceptions.hpp"
#include "utils.hpp"
#include "log.hpp"

namespace caribou {

  //Abstract class for all interfaces
  //@param ADDRESS_T : type for a device address
  //@param REG_T : type for register addresses
  //@param DATA_T : type for data 
  template <typename ADDRESS_T = uint8_t, typename REG_T = uint8_t, typename DATA_T = REG_T >
  class Interface {

  protected:
    Interface(std::string devicePath) : devicePath(devicePath), deviceAddress(0), fixed_address(false) {};
    virtual ~Interface(){};

    //Path of the device
    const std::string devicePath;
    ADDRESS_T deviceAddress;
    bool fixed_address;

    // Provide initial (locked) address
    void lock_address(ADDRESS_T addr) {
      deviceAddress = addr;
      fixed_address = true;
      LOG(logINTERFACE) << "Device address of interface " << devicePath
			<< " locked to " << to_hex_string(deviceAddress);
    }

    template<typename T>
    friend class caribouHAL;

  public:
    //Write data to a device which does not contain internal register
    //If readout is intergralpart of write operations, the read values a returned by function. 
    DATA_T send(const DATA_T& data) { return write(deviceAddress,data); };

        //Write data to a device which does not contain internal register
    //If readout is intergralpart of write operations, the read values a returned by function. 
    std::vector<DATA_T> send(const std::vector<DATA_T>& data) { return write(deviceAddress, data); };

  
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    std::pair<REG_T, DATA_T> send(const std::pair<REG_T, DATA_T> & data) { return write(deviceAddress,data); };
    
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    std::vector<DATA_T> send(const REG_T& reg, const std::vector< DATA_T>& data) { return write(deviceAddress, reg, data); };

  
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    std::vector<std::pair<REG_T, DATA_T> > send(const std::vector<std::pair<REG_T, DATA_T> >& data) { return write(deviceAddress, data); };

    //Read data from a device which does not contain internal register
    std::vector<DATA_T> receive(const unsigned int length = 1) { return read(deviceAddress, length); };

    //Read data from a device containing internal registers
    std::vector<DATA_T> receive(const REG_T reg, const unsigned int length = 1) { return read(deviceAddress, reg, length); };
    
  private:
    //////////////////////
    // Write operations
    //////////////////////

    //Write data to a device which does not contain internal register
    //If readout is intergralpart of write operations, the read values a returned by function. 
    virtual DATA_T write(const ADDRESS_T& address,const DATA_T& data) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    //Write data to a device which does not contain internal register
    //If readout is intergralpart of write operations, the read values a returned by function. 
    virtual std::vector<DATA_T> write(const ADDRESS_T& address, const std::vector<DATA_T>& data) {
      throw CommunicationError("Functionality not provided by this interface");
    };
  
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    virtual std::pair<REG_T, DATA_T> write(const ADDRESS_T& address,const std::pair<REG_T, DATA_T> & data) {
      throw CommunicationError("Functionality not provided by this interface");
    };
    
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    virtual std::vector<DATA_T> write(const ADDRESS_T& address, const REG_T& reg, const std::vector< DATA_T>& data) {
      throw CommunicationError("Functionality not provided by this interface");
    };
  
    //Write data to a device containing internal registers
    //If readout is intergralpart of write operations, the read values a returned by function. 
    virtual std::vector<std::pair<REG_T, DATA_T> > write(const ADDRESS_T& address,const std::vector<std::pair<REG_T, DATA_T> >& data) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    //////////////////////
    // Read operations
    //////////////////////
  
    //Read number of data words form the given device
    virtual std::vector<DATA_T> read(const ADDRESS_T& address, const unsigned int length = 1) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    //Read number of data words form a register of the given device
    virtual std::vector<DATA_T> read(const ADDRESS_T& address, const REG_T reg, const unsigned int length = 1) {
      throw CommunicationError("Functionality not provided by this interface");
    };

  };

}
#endif
