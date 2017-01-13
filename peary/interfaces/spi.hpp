#ifndef CARIBOU_HAL_SPI_H
#define CARIBOU_HAL_SPI_H

#include <vector>
#include <cstdint>
#include <mutex>

namespace caribou {

  /* SPI command interface class
   */
  class iface_spi {

    /** Default constructor: private for singleton class
     */
    iface_spi() {};

  public:
    /** Get instance of the singleton SPI interface class
     *  The below function is thread-safe in C++11 and can thus
     *  be called from several HAL instances concurrently.
     */
    static iface_spi * getInterface() {
      static iface_spi instance;
      return &instance;
    }

    /* Delete unwanted functions from singleton class (C++11)
     */
    iface_spi(iface_spi const&)             = delete;
    void operator=(iface_spi const&)  = delete;

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
    std::vector<uint8_t> sendCommand(uint8_t address, std::vector<uint8_t> data);

  private:

    /** Sending a single SPI command and reading the return value
     */
    uint8_t sendCommand(uint8_t address, uint8_t data);

    /** Private mutex to lock driver access
     */
    std::mutex mutex;
    
  }; //class iface_spi

} //namespace caribou

#endif /* CARIBOU_HAL_SPI_H */
