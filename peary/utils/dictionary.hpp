#ifndef CARIBOU_DICT_H
#define CARIBOU_DICT_H

#include <initializer_list>

#include "exceptions.hpp"
#include "log.hpp"

#include <vector>

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
    registerConfig(DEV_T device, ADDR_T address, SIZE_T size) : _device(device), _address(address), _size(size) {};
    registerConfig(ADDR_T address, SIZE_T size) : _device(DEV_T()), _address(address), _size(size) {};
    DEV_T _device;
    ADDR_T _address;
    SIZE_T _size;
  };
  
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
      return _registers.find(name)->second;
    }

    // Return all registerConfig objects stored by the dictionary:
    std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > getRegisters() const {
      return _registers;
    }

    // Return the register address for the name in question:
    REG_T getAddress(const std::string name) const {
      return _registers.find(name)->second._address;
    }

    // Return the register size for the register in question:
    REG_T getSize(const std::string name) const {
      return _registers.find(name)->second._size;
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

  protected:
    std::map<std::string,registerConfig<REG_T, REG_T, REG_T> > _registers;
  };

} //namespace caribou

#endif /* CARIBOU_DICT_H */
