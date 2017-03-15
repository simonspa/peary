#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <cstdint>

#include "interface.hpp"

namespace caribou {

  class caribouHAL {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL(IFACE interface, std::string device_path, uint32_t device_address);

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getCaRBoardID();

    /** Read value from a firmware register
     *
     *  @param address : address of the register to be read
     */
    uint32_t getFirmwareRegister(uint16_t address);

    /** Get reference to managed device interface
     */
    template<typename T>
    Interface<T> & getInterface();

    /** Read data from managed device interface
     */
    std::vector<uint8_t> read(uint8_t address, uint8_t length);

    
    /** Read the temperature from the TMP101 device
     *
     *  Returns temperature in degree Celsius with a precision of 0.0625degC
     */
    double readTemperature();

    /** Set output voltage of a voltage regulator
     *
     *  The input parameter should be provided in SI Volts and Amps.
     *  The function sets the proper reference voltage (PWR_ADJ_*) in a DAC
     *  corresponding to the regulator. It also configures the corresponding curent/power monitor.
     *  The output of the DAC is not enabled.
     */
    void setVoltageRegulator(const VOLTAGE_REGULATOR_T regulator,const double voltage, const double maxExpectedCurrent);

    /** Set the current source.
     * ~current~ is provided in uA.
     */
    void setCurrentSource(const CURRENT_SOURCE_T source, const unsigned int current,const CURRENT_SOURCE_POLARISATION_T polarisation);
    
    /** Enable/disable the voltage regulator
     *
     *  If enable is true, the function enables first output 
     *  of the coresponding DAC (PWR_ADJ_*). Afterwards, PWER_EN_* is asserted.
     *  If enable is false, the sequence is performed in the opposite direction.
     */
    void powerVoltageRegulator(const VOLTAGE_REGULATOR_T regulator,const bool enable);

    /** Enables current source
     */
    void powerCurrentSource(const CURRENT_SOURCE_T source, const bool enable);

    //The method sets SI5345 jitter attenuator/clock multiplier using a table generated by ClockBuilderPro
    void configureSI5345(SI5345_REG_T const * const regs,const size_t length);


    //The method measures current
    //It return value in SI A.
    double measureCurrent(const VOLTAGE_REGULATOR_T regulator);

    //The method measures power
    //It return value in SI W.
    double measurePower(const VOLTAGE_REGULATOR_T regulator);

    //The method measures voltage
    //It returns vale in SI V.
    double measureVoltage(const VOLTAGE_REGULATOR_T regulator);
    
  private:

    /** Interface of the configured device
     */
    uint8_t _iface;

    /** Device path of the configured device
     */
    std::string _devpath;

    /** Address of the configured device
     */
    uint32_t _devaddress;
    
    /** Set output voltage on a DAC7678 voltage regulator
     *
     *  The input parameter should be provided in SI Volts
     */
    void setDACVoltage(const uint8_t device, const uint8_t address, const double voltage);

    /** Power up/down selected output voltage on a DAC7678 voltage regulator
     */
    void powerDAC(const bool enable, const uint8_t device, const uint8_t address);

    /** Set current/power monitor
     */
    void setCurrentMonitor(const uint8_t device, const double maxExpectedCurrent);

  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
