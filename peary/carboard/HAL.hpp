#ifndef CARIBOU_HAL_H
#define CARIBOU_HAL_H

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "Carboard.hpp"
#include "utils/constants.hpp"
#include "utils/exceptions.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"

#include "interfaces/I2C/i2c.hpp"
#include "interfaces/Memory/memory.hpp"

namespace caribou {

  // Base class to share configuration variable between different caribouHAL specialisations
  class caribouHALbase {
  protected:
    // General reset of the CaR board done
    static bool generalResetDone;
  };

  template <typename T> class caribouHAL : public caribouHALbase {

  public:
    /** Default constructor for creating a new HAL instance
     */
    caribouHAL(std::string device_path, uint32_t device_address);

    /** Default destructor for HAL objects
     */
    ~caribouHAL();

    /** Read and return the device identifier from the firmware
     */
    uint8_t getCaRBoardID();

    /** Return human-readable string of firmware version and build timestamp
     */
    std::string getFirmwareVersion();

    /** Read value from a firmware register
     *
     *  @param address : address of the register to be read
     */
    uint32_t getFirmwareRegister(uint16_t address);

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename T::data_type send(const typename T::data_type& data);

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::pair<typename T::reg_type, typename T::data_type>
    send(const std::pair<typename T::reg_type, typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const typename T::reg_type& reg, const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<std::pair<typename T::reg_type, typename T::data_type>>
    send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data);

    // Read data from a device which does not contain internal register
    std::vector<typename T::data_type> receive(const unsigned int length = 1);

    // Read data from a device containing internal registers
    std::vector<typename T::data_type> receive(const typename T::reg_type reg, const unsigned int length = 1);

    /** Read data from managed device interface
     */
    std::vector<uint8_t> read(uint8_t address, uint8_t length);

    void writeMemory(memory_map mem, size_t offset, uint32_t value);
    void writeMemory(memory_map mem, uint32_t value);
    uint32_t readMemory(memory_map mem, size_t offset);
    uint32_t readMemory(memory_map mem);

    /** Read the temperature from the TMP101 device
     *
     *  Returns temperature in degree Celsius with a precision of 0.0625degC
     */
    double readTemperature();

    /** Set bias voltage
     *
     *  The input parameter should be provided in SI Volts.
     *  The function sets the proper voltage in the DAC corresponding to the selected bias.
     *  The output of the DAC is not enabled.
     */
    void setBiasRegulator(const BIAS_REGULATOR_T regulator, const double voltage);

    /** Enable/disable the voltage regulator
     *
     *  Enables or disables the output of the coresponding DAC.
     */
    void powerBiasRegulator(const BIAS_REGULATOR_T regulator, const bool enable);

    /** Set output voltage of a voltage regulator
     *
     *  The input parameter should be provided in SI Volts and Amps.
     *  The function sets the proper reference voltage (PWR_ADJ_*) in a DAC
     *  corresponding to the regulator. It also configures the corresponding current/power monitor.
     *  The output of the DAC is not enabled.
     */
    void setVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const double voltage, const double maxExpectedCurrent = 3);

    /** Set the current source.
     * ~current~ is provided in uA.
     */
    void
    setCurrentSource(const CURRENT_SOURCE_T source, const unsigned int current, const CURRENT_SOURCE_POLARITY_T polarity);

    /** Enable/disable the voltage regulator
     *
     *  If enable is true, the function enables first output
     *  of the coresponding DAC (PWR_ADJ_*). Afterwards, PWER_EN_* is asserted.
     *  If enable is false, the sequence is performed in the opposite direction.
     */
    void powerVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const bool enable);

    /** Enables current source
     */
    void powerCurrentSource(const CURRENT_SOURCE_T source, const bool enable);

    // The method sets SI5345 jitter attenuator/clock multiplier using a table generated by ClockBuilderPro
    void configureSI5345(SI5345_REG_T const* const regs, const size_t length);

    // The method return true when SI5345 jitter attenuator/clock multiplier is locked
    bool isLockedSI5345();

    // The method sets pulse parameters for an onboard pulser
    void configurePulseParameters(const unsigned channel_mask,
                                  const uint32_t periods,
                                  const uint32_t time_active,
                                  const uint32_t time_idle,
                                  const double voltage);

    // The method configures the onboard pulser
    void configurePulser(unsigned channel_mask, const bool ext_trigger, const bool ext_trig_edge, const bool idle_state);

    // The method starts the onboard pulser
    void startPulser(unsigned channel_mask);

    // The method enables the onboard pulser
    void enablePulser(unsigned channel_mask);

    // The method disables (stops) the onboard pulser
    void disablePulser(unsigned channel_mask);

    // The method returns 4 status bits corresponding to 4 pulser channels if a channel is running
    uint32_t getPulserRunning();

    // The returns the number of pulses that were done since a pulser channel was started for the specified channel.
    uint32_t getPulseCount(const uint32_t channel);

    // The method measures current
    // It return value in SI A.
    double measureCurrent(const VOLTAGE_REGULATOR_T regulator);

    // The method measures power
    // It return value in SI W.
    double measurePower(const VOLTAGE_REGULATOR_T regulator);

    // The method measures voltage
    // It returns vale in SI V.
    double measureVoltage(const VOLTAGE_REGULATOR_T regulator);

    double readSlowADC(const SLOW_ADC_CHANNEL_T channel);

  private:
    // Access to FPGA memory mapped registers
    memory_map reg_firmware{CARIBOU_CONTROL_BASE_ADDRESS,
                            CARIBOU_FIRMWARE_VERSION_OFFSET,
                            CARIBOU_CONTROL_MAP_SIZE,
                            CARIBOU_CONTROL_MAP_MASK,
                            PROT_READ};

    memory_map pulser_control{CARIBOU_PULSER_BASE_ADDRESS,
                              CARIBOU_PULSER_REG_CONTROL_OFFSET,
                              CARIBOU_PULSER_MAP_SIZE,
                              CARIBOU_PULSER_MAP_MASK,
                              PROT_READ | PROT_WRITE};

    memory_map pulser_status{CARIBOU_PULSER_BASE_ADDRESS,
                             CARIBOU_PULSER_REG_STATUS_OFFSET,
                             CARIBOU_PULSER_MAP_SIZE,
                             CARIBOU_PULSER_MAP_MASK,
                             PROT_READ};
    memory_map pulser_counts{CARIBOU_PULSER_BASE_ADDRESS,
                             CARIBOU_PULSER_REG_PULSE_COUNT_OFFSET,
                             CARIBOU_PULSER_MAP_SIZE,
                             CARIBOU_PULSER_MAP_MASK,
                             PROT_READ};
    memory_map pulser_periods{CARIBOU_PULSER_BASE_ADDRESS,
                              CARIBOU_PULSER_REG_PERIODS_OFFSET,
                              CARIBOU_PULSER_MAP_SIZE,
                              CARIBOU_PULSER_MAP_MASK,
                              PROT_READ | PROT_WRITE};
    memory_map pulser_time_active{CARIBOU_PULSER_BASE_ADDRESS,
                                  CARIBOU_PULSER_REG_TIME_ACTIVE_OFFSET,
                                  CARIBOU_PULSER_MAP_SIZE,
                                  CARIBOU_PULSER_MAP_MASK,
                                  PROT_READ | PROT_WRITE};
    memory_map pulser_time_idle{CARIBOU_PULSER_BASE_ADDRESS,
                                CARIBOU_PULSER_REG_TIME_IDLE_OFFSET,
                                CARIBOU_PULSER_MAP_SIZE,
                                CARIBOU_PULSER_MAP_MASK,
                                PROT_READ | PROT_WRITE};

    // It's not possible to power on/off DCDC converter by software
    void setDCDCConverter(const DCDC_CONVERTER_T converter, const double voltage);

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

    /** General reset of the CaR board
     */
    void generalReset();

  }; // class caribouHAL

} // namespace caribou

#include "HAL.tcc"

#endif /* CARIBOU_HAL_H */
