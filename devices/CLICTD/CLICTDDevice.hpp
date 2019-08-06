/**
 * Caribou implementation for the CLICTD chip
 */

#ifndef DEVICE_CLICTD_H
#define DEVICE_CLICTD_H

#include "device/CaribouDevice.hpp"
#include "interfaces/I2C/i2c.hpp"

#include "clockgenerator/Si5345-RevB-CLICTD-Registers.h"

#include "CLICTDDefaults.hpp"
#include "CLICTDPixels.hpp"

namespace caribou {

  /** CLICTD Device class definition
   */
  class CLICTDDevice : public CaribouDevice<iface_i2c> {

  public:
    CLICTDDevice(const caribou::Configuration config);
    ~CLICTDDevice();

    /** Initializer function for CLICTD
     */
    void configure();

    /** Turn on the power supply for the CLICTD chip
     */
    void powerUp();

    /** Turn off the CLICTD power
     */
    void powerDown();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    // Reset the chip
    void reset();

    pearydata getData();
    std::vector<uint32_t> getRawData();

    void setSpecialRegister(std::string name, uint32_t value);
    uint32_t getSpecialRegister(std::string name);

    void configureMatrix(std::string filename);

    void configureClock(bool internal = 1);

    void getMem(std::string);
    void setMem(std::string, uint32_t value);

    bool storeFrame();
    /**
     * @brief Set correct channel for output multiplexer
     */
    void setOutputMultiplexer(std::string name);

  private:
    /**
     * Routine to program the pixel matrix
     * This routine produces a bit matrix (using STL vector<bool>) which can directly be sent to the ASIC
     */
    void programMatrix();

    /* Map of pixelConfigs for configuration storage (column, row))
     */
    using matrixConfig = std::map<std::pair<uint8_t, uint8_t>, std::pair<pixelConfigStage1, pixelConfigStage2>>;
    matrixConfig pixelConfiguration;

    matrixConfig readMatrix(std::string filename) const;
  };

} // namespace caribou

#endif /* DEVICE_CLICTD_H */
