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
  class registerConfig {
  public:
    registerConfig() {};
    registerConfig(uint8_t address, uint8_t size) : _address(address), _size(size) {};
    uint8_t _address;
    uint8_t _size;
  };
  
  /** Dictionary class for generic register name lookup
   *  All register names are lower case, register selection is case-insensitive.
   *  Singleton class, only one object of this floating around.
   */
  template <typename T>
  class dictionary {
  public:
    dictionary() {};
    ~dictionary() {};

    // Return the register address for the name in question:
    static inline uint8_t getAddress(const std::string name) {
      return _registers.find(name)->second._address;
    }

    // Return the register size for the register in question:
    static inline uint8_t getSize(const std::string name) {
      return _registers.find(name)->second._size;
    }

    // Return the register size for the register in question:
    static inline uint8_t getSize(const uint8_t address) {
      for(auto reg : _registers) {
	if(reg.second._address == address) {
	  return reg.second._size;
	}
      }
      throw UndefinedRegister("Register not found");
    }

    // Return the register name for the register in question:
    static inline std::string getName(const uint8_t address) {
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
    static std::map<std::string,registerConfig> _registers;
  };

  template <typename T> std::map<std::string,registerConfig> dictionary<T>::_registers;
} //namespace caribou

#endif /* CARIBOU_DICT_H */
