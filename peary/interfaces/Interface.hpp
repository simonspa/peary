#ifndef CARIBOU_HAL_INTERFACE_HPP
#define CARIBOU_HAL_INTERFACE_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "utils/exceptions.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace caribou {

  // Abstract class for all interfaces
  //@param ADDRESS_T : type for a device address
  //@param REG_T : type for register addresses
  //@param DATA_T : type for data
  template <typename ADDRESS_T = uint8_t, typename REG_T = uint8_t, typename DATA_T = REG_T> class Interface {
  public:
    typedef ADDRESS_T addr_type;
    typedef REG_T reg_type;
    typedef DATA_T data_type;

    std::string devicePath() const { return _devicePath; }

  protected:
    Interface(std::string devicePath) : _devicePath(devicePath){};
    virtual ~Interface(){};

    // Path of the device
    const std::string _devicePath;

    //////////////////////
    // Write operations
    //////////////////////

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual DATA_T write(const ADDRESS_T&, const DATA_T&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::vector<DATA_T> write(const ADDRESS_T&, const std::vector<DATA_T>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::pair<REG_T, DATA_T> write(const ADDRESS_T&, const std::pair<REG_T, DATA_T>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::vector<DATA_T> write(const ADDRESS_T&, const REG_T&, const std::vector<DATA_T>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::vector<std::pair<REG_T, DATA_T>> write(const ADDRESS_T&, const std::vector<std::pair<REG_T, DATA_T>>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    //////////////////////
    // Read operations
    //////////////////////

    // Read number of data words form the given device
    virtual std::vector<DATA_T> read(const ADDRESS_T&, const unsigned int = 1) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Read number of data words form a register of the given device
    virtual std::vector<DATA_T> read(const ADDRESS_T&, const REG_T, const unsigned int = 1) {
      throw CommunicationError("Functionality not provided by this interface");
    };
  };

} // namespace caribou
#endif
