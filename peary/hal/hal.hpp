#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <vector>
#include <map>
#include <tuple>
#include <string>
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
    uint8_t getCaRBoardID();

    /** Read value from a firmware register
     *
     *  @param address : address of the register to be read
     */
    uint32_t getFirmwareRegister(uint16_t address);
    
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

    /** Set output voltage of a voltage regulator
     *
     *  The input parameter should be provided in SI Volts.
     *  The function sets the proper reference voltage (PWR_ADJ_*) in a DAC
     *  corresponding to the regulator. 
     *  The output of the DAC is not enabled.
     */
    void setVoltageRegulator(VOLTAGE_REGULATOR_T regulator, double voltage);

    /** Enable/disable the voltage regulator
     *
     *  If enable is true, the function enables first output 
     *  of the coresponding DAC (PWR_ADJ_*). Afterwards, PWER_EN_* is asserted.
     *  If enable is false, the sequence is performed in the opposite direction.
     */
    void powerVoltageRegulator(VOLTAGE_REGULATOR_T regulator, bool enable);

    //FIXME: User shouldn't have access to set setDACVoltage and powerDAC where he can
    //       provide any I2C address. Instead enums should be used to a specialised functions,
    //       like setBias, setInjectionBias, setCurrent, etc.


  private:

    static std::map< VOLTAGE_REGULATOR_T, std::tuple<uint8_t,uint8_t, uint8_t, std::string> > create_map();
    //the tuple stores (in the order) the coressponding: DAC pin, pin of the port 1 of the U15, I2C address of the current/power monitor and PWR name
    static const std::map< VOLTAGE_REGULATOR_T, std::tuple<uint8_t,uint8_t, uint8_t, std::string> > voltageRegulatorMap;
    
    /** Interface of the configured device
     */
    uint8_t _iface;

    /** Device path of the configured device
     */
    std::string _devpath;
    
    /** Set output voltage on a DAC7678 voltage regulator
     *
     *  The input parameter should be provided in SI Volts
     */
    void setDACVoltage(uint8_t device, uint8_t address, double voltage);

    /** Power up/down selected output voltage on a DAC7678 voltage regulator
     */
    void powerDAC(bool enable, uint8_t device, uint8_t address);


  }; //class caribouHAL

} //namespace caribou

#endif /* CARIBOU_HAL_H */
