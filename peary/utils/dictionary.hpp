#ifndef CARIBOU_DICT_H
#define CARIBOU_DICT_H

#include <initializer_list>
#include <iostream>
#include <vector>
#include <map>

#include "exceptions.hpp"
#include "constants.hpp"
#include "log.hpp"

namespace caribou {

  /** class to store a register or DAC configuration
   *
   *  Contains register address, size of the register
   *  @param 
   */
  template<typename DEV_T = uint8_t, typename ADDR_T = uint8_t, typename SIZE_T = uint8_t>
  class registerConfig {
  public:
    registerConfig() {};
    registerConfig(DEV_T device, ADDR_T address, SIZE_T size, REGTYPE type = UNDEFINED) : _device(device), _address(address), _size(size), _type(type) {};
    registerConfig(ADDR_T address, SIZE_T size) : _device(DEV_T()), _address(address), _size(size), _type(UNDEFINED) {};
    DEV_T _device;
    ADDR_T _address;
    SIZE_T _size;
    REGTYPE _type;

    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, const registerConfig<T, T, T>& rg);
  };

  template<typename T>
  std::ostream& operator<<(std::ostream& os, const caribou::registerConfig<T, T, T>& rg) {  
    os << to_hex_string(rg._device) << ":"
       << to_hex_string(rg._address) << ", "
       << static_cast<int>(rg._size) << ", "
       << static_cast<int>(rg._type);  
    return os;  
  }
  

  /** Dictionary class for generic register name lookup
   *  All register names are lower case, register selection is case-insensitive.
   */
  template <typename REG_T = uint8_t>
  class dictionary {
  public:
    dictionary() {};
    dictionary(std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > reg) { _registers = reg; };
    dictionary(std::initializer_list<std::pair<std::string const,registerConfig<REG_T, REG_T, REG_T> > > reg) : _registers(reg) {};
    ~dictionary() {};

    // Return the register object for the name in question:
    registerConfig<REG_T, REG_T, REG_T> getRegister(const std::string name) const {
      try {
	return _registers.at(name);
      }
      catch(...) {
	throw UndefinedRegister("Register not found");
      }      
    }

    // Return all registerConfig objects stored by the dictionary:
    std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > getRegisters() const {
      return _registers;
    }

    // Return the device for the name in question:
    REG_T getDevice(const std::string name) const {
      try {
	return _registers.at(name)._device;
      }
      catch(...) {
	throw UndefinedRegister("Register not found");
      }      
    }

    // Return the register address for the name in question:
    REG_T getAddress(const std::string name) const {
      try {
	return _registers.at(name)._address;
      }
      catch(...) {
	throw UndefinedRegister("Register not found");
      }      
    }

    // Return the register size for the register in question:
    REG_T getSize(const std::string name) const {
      try {
	return _registers.at(name)._size;
      }
      catch(...) {
	throw UndefinedRegister("Register not found");
      }      
    }

    // Return the register size for the register in question:
    REG_T getSize(const REG_T address) const {
      for(auto reg : _registers) {
	if(reg.second._address == address) {
	  return reg.second._size;
	}
      }
      throw UndefinedRegister("Register not found");
    }

    // Return the register name for the register in question:
    std::string getName(const REG_T address) const {
      for(auto reg : _registers) {
	if(reg.second._address == address) {
	  return reg.first;
	}
      }
      throw UndefinedRegister("Register not found");
    }

    // Return all register names for the type in question:
    std::vector<std::string> getNames() const {
      std::vector<std::string> names;
      for(auto reg : _registers) {
	  names.push_back(reg.first);
      }
      return names;
    }

    // Allow merging another dictionary 
    dictionary<REG_T>& operator += (const dictionary<REG_T> &obj) {
      std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > reg = obj.getRegisters();
      _registers.insert(reg.begin(), reg.end());
      return (*this) ;
    }

    void dump(const TLogLevel level = logINFO, const REGTYPE = UNDEFINED) {
      for(auto reg : _registers) {
	LOG(level) << reg.first << ": " << reg.second;
      }
    }

  protected:
    std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > _registers;
  };

} //namespace caribou

#endif /* CARIBOU_DICT_H */
