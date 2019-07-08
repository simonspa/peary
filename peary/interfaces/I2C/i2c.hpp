#ifndef CARIBOU_HAL_I2C_HPP
#define CARIBOU_HAL_I2C_HPP

#include <mutex>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  typedef uint8_t i2c_address_t;
  typedef uint8_t i2c_t;
  typedef uint8_t i2c_reg_t;

  /**
   * @ingroup Interfaces
   * @brief I2C interface via Kernel I2C module
   */
  class iface_i2c : public Interface<i2c_t, i2c_reg_t, i2c_t> {
  private:
    /**
     * @brief Private constructor, only to be created by the interface manager. Opens the file descriptor for the I2C module
     * @param device_path Path the device listens at
     * @throws DeviceException if device cannot be reached
     */
    iface_i2c(std::string const& device_path);

    /**
     * @brief Default constructor deleted, cannot have I2C interface without bus address
     */
    iface_i2c() = delete;

    /**
     * @brief Destructor closes the file handle for the I2C module
     */
    ~iface_i2c();

    /**
     * @brief Sets endpoint address before communication
     * @param address I2C address to address
     * @throws CommunicationError if device cannot be contacted
     */
    inline void setAddress(i2c_address_t const address);

    // File descriptor for the I2C module
    int i2cDesc;

    // Protects access to the bus
    std::mutex mutex;

    // caribouHAL is allowed to access private members for reading and writing
    template <typename T> friend class caribouHAL;

  public:
    i2c_t write(const i2c_address_t& address, const i2c_t& data);
    std::pair<i2c_reg_t, i2c_t> write(const i2c_address_t& address, const std::pair<i2c_reg_t, i2c_t>& data);
    std::vector<i2c_t> write(const i2c_address_t& address, const i2c_reg_t& reg, const std::vector<i2c_t>& data);

    // length must be 1
    std::vector<i2c_t> read(const i2c_address_t& address, const unsigned int length = 1);
    // length must be 32
    std::vector<i2c_t> read(const i2c_address_t& address, const i2c_reg_t reg, const unsigned int length = 32);

  private:
    // Special functions to read/write to devices with up to 16bit register
    std::vector<i2c_t> wordwrite(const i2c_address_t& address, const uint16_t& reg, const std::vector<i2c_t>& data);
    std::vector<i2c_t> wordread(const i2c_address_t& address, const uint16_t reg, const unsigned int length = 1);

    // only this function can create the interface
    friend iface_i2c& InterfaceManager::getInterface<iface_i2c>(std::string const&);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_I2C_HPP */
