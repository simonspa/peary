#ifndef CARIBOU_HAL_SPI_H
#define CARIBOU_HAL_SPI_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "interface.hpp"
#include "InterfaceManager.hpp"

namespace caribou {

  typedef uint8_t spi_address_t; // is ignored
  typedef uint8_t spi_t;
  typedef uint8_t spi_reg_t;

  /* SPI command interface class
   */
  class iface_spi : public Interface<spi_address_t, spi_reg_t, spi_t> {

  protected:
    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    iface_spi(std::string const& device_path);

    virtual ~iface_spi();

    // Descriptor of the device
    int spiDesc;

    // Protects access to the bus
    std::mutex mutex;

    // SPI mode.
    // Default: 0 (SPI_CPHA = 0, SPI_CPOL = 0)
    const uint32_t mode = 0x0;

    // Maximum speed in Hz
    // Default: 100 MHz
    const uint32_t speed = 100000000;

    template <typename T> friend class caribouHAL;

  protected:
    std::pair<spi_reg_t, spi_t> write(const spi_address_t& address, const std::pair<spi_reg_t, spi_t>& data);
    std::vector<spi_t> write(const spi_address_t& address, const spi_t& reg, const std::vector<spi_t>& data);
    std::vector<std::pair<spi_reg_t, spi_t>> write(const spi_address_t& address,
                                                   const std::vector<std::pair<spi_reg_t, spi_t>>& data);
    std::vector<spi_t> read(const spi_address_t& address, const spi_reg_t reg, const unsigned int length = 1);

    // Unused constructor
    iface_spi() = delete;

    // only this function can create the interface
    friend iface_spi& InterfaceManager::getInterface<iface_spi>(std::string const&);
  }; // class iface_spi

} // namespace caribou

#endif /* CARIBOU_HAL_SPI_H */
