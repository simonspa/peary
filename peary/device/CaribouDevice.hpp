/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_MIDDLEWARE_H
#define CARIBOU_MIDDLEWARE_H

#include "Device.hpp"
#include "utils/configuration.hpp"
#include "utils/constants.hpp"
#include "utils/dictionary.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace caribou {

  /** Forward declaration of the hardware abstraction layer, not including the header file!
   */
  template <typename T> class caribouHAL;

  /** Caribou Device class definition
   *
   *  this is the central device class from which all device implementations inherit.
   *
   *  Some basic functionality is defined via purely virtual member functions which
   *  have to be implemented by every device instance. This enables the possibility
   *  of interfacing the devices independently via the common set of function alls, e.g.,
   *  from a GUI or a commandline interface.
   */
  template <typename T> class CaribouDevice : public Device {

  public:
    /** Default constructor for Caribou devices
     *
     */
    CaribouDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr = 0);

    /** Default destructor for Caribou devices
     */
    virtual ~CaribouDevice();

    /** Return the firmware version string for reference
     */
    std::string getFirmwareVersion();

    /** Return the human-readable device name
     */
    virtual std::string getName() { return getType(); };

    /** Return the human-readable device type
     */
    std::string getType();

    /** Return the identifier of the firmware currently loaded
     */
    uint8_t getFirmwareID();

    /** Return the board identifier of the CaR board installed
     */
    uint8_t getCaRBoardID();

    /** Read the ID from the chip board if available
     *
     *  Some chip boards feature an EPROM which stores a board ID and thus
     *  allows identification of the attached chip board.
     */
    uint16_t getChipID() { return 0; };

    /** Return the human-readable device name of the firmware currently loaded
     */
    std::string getDeviceName();

    /** Call the device's powerUp() function and toggle the powered state
     */
    void powerOn();

    /** Turn on the power supply for the attached device
     */
    virtual void powerUp() = 0;

    /** Call the device's powerDown() function and toggle the powered state
     */
    void powerOff();

    /** Turn off power for the attached device
     */
    virtual void powerDown() = 0;

    /** Start the data acquisition
     */
    virtual void daqStart() = 0;

    /** Stop the data acquisition
     */
    virtual void daqStop() = 0;

    virtual void configure();

    // Controlling the device

    /**
     * @brief Set a register on this device
     * @param name  Name of the register
     * @param value Value to be set
     */
    void setRegister(std::string name, uint32_t value);
    virtual void setSpecialRegister(std::string, uint32_t){};
    virtual uint32_t getSpecialRegister(std::string) { return 0; };
    uint32_t getRegister(std::string name);
    std::vector<std::pair<std::string, uint32_t>> getRegisters();

    /** Sending reset signal to the device
     */
    virtual void reset();

    // Setting the acquisition clock/device clock?
    // Could be either the supplied clock from DAQ or internal clock divider...
    // virtual void setClockFrequency();

    // Voltage regulators

    // To set supply voltages, same question as above: how to define voltage names?
    // Separate functions to set target voltage and activate?
    // Purely virtual?
    // Do they need to be virtual? Maybe implement in baseclass (always same for CaR)
    // and only look up correct regulator according to name from child class dictionary?
    void setVoltage(std::string name, double voltage, double currentlimit = 3);
    void setCurrent(std::string name, int current, bool polarity);

    void switchOn(std::string name) { return switchPeripheryComponent(name, true); };
    void switchOff(std::string name) { return switchPeripheryComponent(name, false); };

    double getVoltage(std::string name);
    double getCurrent(std::string name);
    double getPower(std::string name);

    // virtual double getTemperature();

    /** Read slow-ADC value by name of the input signal as defined by the device
     *
     *  Returns value in SI Volts
     */
    double getADC(std::string name);

    std::vector<std::string> listRegisters();
    std::vector<std::pair<std::string, std::string>> listComponents();

    /** Read slow-ADC value by the input channel number of the ADC device
     *
     *  Returns value in SI Volts
     */
    double getADC(uint8_t channel);

    // CaR CMOS signals
    // void enableSignal();
    // void disableSignal();

    // Retrieving data

    // Raw and decoded data readback
    std::vector<uint32_t> getRawData();
    pearydata getData();

    // Two types:
    //  * trigger based: "events" are returned
    //  * shutter based: "frames" are returned
    // Both contain pixel(s), timestamp(s)
    // virtual std::vector<caribou::event> getData();
    // If no data available, throw caribou::NoDataAvailable exception instead of returning empty vector!
    // Otherwise synchronization of event-based detectors impossible

    void setMemory(std::string name, size_t offset, uint32_t value);
    void setMemory(std::string name, uint32_t value);
    uint32_t getMemory(std::string name, size_t offset);
    uint32_t getMemory(std::string name);

  protected:
    /**
     * @brief process registers, ingoring sepcial flags
     * @param name  Name of the registers
     * @param value Value of the register to be set
     */
    void process_register(register_t<typename T::reg_type, typename T::data_type> reg, std::string name, uint32_t value);

    /** Instance of the Caribou hardware abstraction layer library
     *
     *  All register and hardware access should go through this interface.
     */
    caribou::caribouHAL<T>* _hal;

    /** Device configuration object
     */
    caribou::Configuration _config;

    /** Register dictionary for the devices:
     */
    caribou::dictionary<register_t<typename T::reg_type, typename T::data_type>> _registers;

    /** Register cache
     */
    std::map<std::string, typename T::data_type> _register_cache;

    /** Periphery dictionary to access CaR components:
     */
    caribou::dictionary<component_t> _periphery;

    /** Memory page dictionary to access FPGA registers:
     */
    caribou::dictionary<memory_map> _memory;

  private:
    /** State indicating powering of the device
     */
    bool _is_powered;

    /** State indicating the configuration of the device
     */
    bool _is_configured;

    /** Switcher function for periphery component (turns them on/off)
     */
    void switchPeripheryComponent(std::string name, bool enable);

  }; // class CaribouDevice

} // namespace caribou

#include "CaribouDevice.tcc"

#endif /* CARIBOU_MIDDLEWARE_H */
