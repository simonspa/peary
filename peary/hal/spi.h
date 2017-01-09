#ifndef CARIBOU_HAL_SPI_H
#define CARIBOU_HAL_SPI_H

#include <vector>
#include <cstdint>

namespace caribou {

  /* SPI command interface class
   */
  class spi {

  public:
    /** Default constructor for creating a new SPI communicator
     */
    spi() {};

    /** Default destructor for SPI objects
     */
    ~spi() {};

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

  }; //class SPI

} //namespace caribou

#endif /* CARIBOU_HAL_SPI_H */
