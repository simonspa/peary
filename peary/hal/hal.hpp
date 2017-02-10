#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>
#include <cstdint>

namespace caribou {

  class caribouHAL {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL(IFACE interface, std::string device_path);

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getDeviceID();

    /** Send command to the managed device interface
     */
    std::vector<uint8_t> write(std::vector<uint8_t> address, std::vector<uint8_t> data);
    std::vector<uint8_t> write(const uint8_t address, const std::vector<uint8_t> & data);

    /** Read data from managed device interface
     */
    std::vector<uint8_t> read(uint8_t address, uint8_t length);

    
    /** Read the temperature from the TMP101 device
     *
     *  Returns temperature in degree Celsius with a precision of 0.0625degC
     */
    double readTemperature();

    /** Set output voltage on a DAC7678 voltage regulator
     *
     *  The input parameter should be provided in SI Volts
     */
    void setVoltage(uint8_t address, double voltage);
    
  private:

    /** Interface of the configured device
     */
    uint8_t _iface;

    /** Device path of the configured device
     */
    std::string _devpath;

  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
