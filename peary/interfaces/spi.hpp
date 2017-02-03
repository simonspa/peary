#ifndef CARIBOU_HAL_SPI_H
#define CARIBOU_HAL_SPI_H

#include <vector>
#include <cstdint>
#include <string>
#include <mutex>

#include "exceptions.hpp"
#include "interface_manager.hpp"
#include "interface.hpp"

namespace caribou {

  typedef uint8_t spi_t;

  /* SPI command interface class
   */
  class iface_spi : public Interface<spi_t, spi_t> {

  private:
    //Default constructor: private (only created by interface_manager)
    //It can throw DeviceException
    iface_spi(std::string const & device_path);

  public:
    /** Send command via the SPI interface
     *
     *  The function accepts the register address to be written to as well as
     *  a vector of data words to be sent (MOSI, master out slave in).
     *  All data is send to the same address.
     *
     *  If the data vector is empty, no SPI command will be sent.
     *
     *  The return value is the response retrieved from the SPI interface 
     *  (MISO, master in slave out)
     *  The responses from each individual SPI command are returned in the order
     *  of the command execution.
     *  For SPI commands which do not correspond to a MISO output, the return
     *  vector is empty.
     */
    std::vector<spi_t> write(const spi_t& address, const spi_t& data );
    std::vector<spi_t> write(const spi_t& address, const std::vector<spi_t>& data );
    std::vector<spi_t> write(const spi_t& address,const std::pair<spi_t, spi_t> & data);
    std::vector<spi_t> write(const spi_t& address, const spi_t & reg, const std::vector< spi_t > & data);
    std::vector<spi_t> write(const spi_t& address,const std::vector< std::pair<spi_t, spi_t> > & data);
    std::vector<spi_t> read(const spi_t& address, const unsigned int& length);
    std::vector<spi_t> read(const spi_t& address, const spi_t reg, const unsigned int& length = 1);

  private:

    /** Sending a single SPI command and reading the return value
     */
    uint8_t sendCommand(uint8_t address, uint8_t data);

    /** Private mutex to lock driver access
     */
    std::mutex mutex;

    //only this function can create the interface
    friend iface_spi& interface_manager::getInterface< iface_spi >(std::string const & );
  }; //class iface_spi

} //namespace caribou

#endif /* CARIBOU_HAL_SPI_H */
