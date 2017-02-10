#ifndef CARIBOU_DICT_H
#define CARIBOU_DICT_H

#include "exceptions.hpp"
#include "log.hpp"

#include <vector>

namespace caribou {

  /** class to store a register or DAC configuration
   *
   *  Contains register address, size of the register
   *  @param 
   */
  template<typename T = uint8_t>
  class registerConfig {
  public:
    registerConfig() {};
    registerConfig(T address, T size) : _address(address), _size(size) {};
    T _address;
    T _size;
  };
  
  /** Dictionary class for generic register name lookup
   *  All register names are lower case, register selection is case-insensitive.
   *  Singleton class, only one object of this floating around.
   */
  template <typename DICT_T, typename REG_T = uint8_t>
  class dictionary {
  public:
    dictionary() {};
    ~dictionary() {};

    // Return the register address for the name in question:
    static inline REG_T getAddress(const std::string name) {
      return _registers.find(name)->second._address;
    }

    // Return the register size for the register in question:
    static inline REG_T getSize(const std::string name) {
      return _registers.find(name)->second._size;
    }

    // Return the register size for the register in question:
    static inline REG_T getSize(const REG_T address) {
      for(auto reg : _registers) {
	if(reg.second._address == address) {
	  return reg.second._size;
	}
      }
      throw UndefinedRegister("Register not found");
    }

    // Return the register name for the register in question:
    static inline std::string getName(const REG_T address) {
      for(auto reg : _registers) {
	if(reg.second._address == address) {
	  return reg.first;
	}
      }
      throw UndefinedRegister("Register not found");
    }

    // Return all register names for the type in question:
    static inline std::vector<std::string> getNames() {
      std::vector<std::string> names;
      for(auto reg : _registers) {
	  names.push_back(reg.first);
      }
      return names;
    }

  protected:
    static std::map<std::string,registerConfig<REG_T> > _registers;
  };

  template <typename DICT_T, typename REG_T> std::map<std::string,registerConfig<REG_T> > dictionary<DICT_T, REG_T>::_registers;
} //namespace caribou

#endif /* CARIBOU_DICT_H */
