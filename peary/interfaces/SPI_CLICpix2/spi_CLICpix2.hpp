#ifndef CARIBOU_HAL_SPI_CLICPIX2_H
#define CARIBOU_HAL_SPI_CLICPIX2_H

#include "interfaces/SPI/spi.hpp"

namespace caribou {

  /* SPI command interface class
   */
  class iface_spi_CLICpix2 : public iface_spi {

  protected:
    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    iface_spi_CLICpix2(std::string const& device_path) : iface_spi(device_path){};

    virtual ~iface_spi_CLICpix2(){};

    template <typename T> friend class caribouHAL;

    std::pair<spi_reg_t, spi_t> write(const spi_address_t& address, const std::pair<spi_reg_t, spi_t>& data);
    std::vector<std::pair<spi_reg_t, spi_t>> write(const spi_address_t& address,
                                                   const std::vector<std::pair<spi_reg_t, spi_t>>& data);

    // Unused constructor
    iface_spi_CLICpix2() = delete;

    // only this function can create the interface
    friend iface_spi_CLICpix2& InterfaceManager::getInterface<iface_spi_CLICpix2>(std::string const&);
  }; // class iface_spi_CLICpix2

} // namespace caribou

#endif /* CARIBOU_HAL_SPI_CLICPIX2_H */
