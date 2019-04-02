/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_API_H
#define CARIBOU_API_H

#include "configuration.hpp"
#include "constants.hpp"
#include "datatypes.hpp"
#include "dispatcher.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace caribou {

  /**
   * @brief Abstract Caribou Device class definition
   *
   * This is the central device class from which all device implementations inherit. Some basic functionality is defined via
   * purely virtual member functions which have to be implemented by every device instance. This enables the possibility of
   * interfacing the devices independently via the common set of function alls, e.g., from a GUI or a commandline interface.
   */
  class caribouDevice {

  public:
    /**
     * @brief Default constructor for Caribou devices
     * @param caribou::Configuration Configuration object
     */
    caribouDevice(const caribou::Configuration);

    /**
     * @brief Default destructor for Caribou devices
     */
    virtual ~caribouDevice(){};

    /**
     * @brief Indicator flag for managed devices
     * @return True if managed, false if unmanaged
     */
    bool isManaged() { return managedDevice; };

    /**
     * @brief Return the software version string for reference
     * @brief Peary software version string
     */
    std::string getVersion();

    /**
     * @brief Return the version string of the firmware currently loaded
     * @return Firmware version string
     */
    virtual std::string getFirmwareVersion() = 0;

    /**
     * @brief Return the human-readable device name (full qualifier)
     * @return Device name
     */
    virtual std::string getName() = 0;

    /**
     * @brief Return the device type, derived from the class name
     * @return Device type
     */
    virtual std::string getType() = 0;

    /**
     * @brief Turn on all registered power supplies for the device
     */
    virtual void powerOn() = 0;

    /**
     * @brief Turn off all registered power supplies for the device
     */
    virtual void powerOff() = 0;

    /**
     * @brief Start data acquisition mode
     */
    virtual void daqStart() = 0;

    /**
     * @brief Stop data acquisition mode
     */
    virtual void daqStop() = 0;

    /**
     * @brief Retrieve raw data from the device.
     *
     * This data is a vector of 32bit integers of undecoded detector response. Peary does not make any assumption on the
     * content or structure, and special decoders have to be provided in order to decode and interpret this data. This is the
     * method to be used by integrated data acquisition systems such as EUDAQ to retrieve data from the attached detector
     * without the requirement of knowing which exact device it is.
     *
     * @return Raw detector data
     */
    virtual std::vector<uint32_t> getRawData() = 0;

    /**
     * @brief Retrieve decoded data from the device.
     *
     * This method is designed for immediate quality checks of the data. Each device has to implement a decoder to translate
     * their own data into a readable format. For this propose, the caribou::pearydata data type is used, which is a map of
     * pixel coordinates and unique pointers to pixel objects. These pixel objects can be overloaded by the device to store
     * relevant information. The method can also be used to e.g. write decoded data directly into an ASCII file.
     *
     * @return Decoded detector data
     */
    virtual pearydata getData() = 0;

    /**
     * @brief Configure the device
     *
     * Initialize the device (ex.set the required clock otuputs etc.). This function is part of the standard "booting"
     * sequence for any device and should be called by programs using the Peary library for cdevice communication. Among
     * other settings, it configures the with all DACs to the values provided via the initial configuration object passed to
     * the device constructor.
     */
    virtual void configure() = 0;

    /**
    * @brief Set register on the device
    *
    * The register is identified by its human-readable name using the register dictionary, its value is automatically casted
    * to the register data type (e.g. 8-bit). The device has to provide a list of valid register names.
    *
    * @param name  Name of the register
    * @param value Value to be programmed
    * @throws ConfigInvalid if the register name is not valid
    * @throws RegisterTypeMismatch if the register is not writable
    * @throws CommunicationError if the device could not be contacted
    */
    virtual void setRegister(std::string name, uint32_t value) = 0;

    /**
     * @brief Get register from the device
     *
     * Retrieve content of specified register from the device. The register is identified by its human-readable name via the
     * register dictionary.
     *
     * @param  name Name of the register
     * @return      Value of the register
     * @throws ConfigInvalid if the register name is not valid
     * @throws RegisterTypeMismatch if the register is not readable
     * @throws CommunicationError if the device could not be contacted
     */
    virtual uint32_t getRegister(std::string name) = 0;

    /**
     * @brief Get list of all readable registers with their current values
     *
     * This method internally calls device::getRegister for every known register of the device. Only readable registers are
     * returned and no exception is thrown when attempting to read write-only registers.
     *
     * @returns Vector with pairs of register name and value
     */
    virtual std::vector<std::pair<std::string, uint32_t>> getRegisters() = 0;

    /**
     * @brief Send reset signal to the device
     */
    virtual void reset() = 0;

    /**
     * @brief Set voltage and current limit on CaR components
     *
     * This method allows to configure voltage regulators, bias voltages and injection pulser voltages
     * @param name         Name of the component
     * @param voltage      Voltage to be set in V
     * @param currentlimit Optional current limit to set in A, not used by all components
     */
    virtual void setVoltage(std::string name, double voltage, double currentlimit = 3) = 0;

    /**
     * @brief Switch on the given CaR resource
     *
     * This method allows to switch on individual resources on the CaR board such as power supplied or biad voltages. The
     * resource is identified by its human-readable name and has to be registered by the device in the periphery dictionary
     *
     * @param name Name of the resource
     */
    virtual void switchOn(std::string name) = 0;

    /**
     * @brief Switch off the given CaR resource
     *
     * This method allows to switch off individual resources on the CaR board. The resource is identified by its
     * human-readable name and has to be registered by the device in the periphery dictionary.
     *
     * @param name Name of the resource
     */
    virtual void switchOff(std::string name) = 0;

    /**
     * @brief Configure CaR current source
     * @param name     Name of the current source
     * @param current  Current to be set in A
     * @param polarity Polarity of the source, true: PUSH, false: PULL
     */
    virtual void setCurrent(std::string name, int current, bool polarity) = 0;

    virtual double getVoltage(std::string name) = 0;
    virtual double getCurrent(std::string name) = 0;
    virtual double getPower(std::string name) = 0;

    /**
     * @brief Read ADC voltage value
     *
     * The input is identified by its name as defined by the device
     *
     * @returns Samples voltage in units of V
     * @throws ConfigInvalid if the signal name is invalid
     */
    virtual double getADC(std::string name) = 0;

    /**
     * @brief Read ADC voltage value
     *
     * The input is identified by the channel number of the ADC chip
     *
     * @returns Samples voltage in units of V
     * @throws ConfigInvalid if the channel number does not exist
     */
    virtual double getADC(uint8_t channel) = 0;

    /**
     * @brief Retrieve list of all available device-specifiv commands
     * @return List of available commands as pair of command name and number of expected arguments
     */
    std::vector<std::pair<std::string, std::size_t>> listCommands();

    /**
     * @brief Call device-specific command with a list of arguments
     *
     * @return String containing the return value of the command
     * @throws ConfigInvalid if command is not found or number of arguments does not match
     */
    std::string command(const std::string& name, const std::vector<std::string>& args = std::vector<std::string>());

    /**
     * @brief Call device-specific command with a single argument
     *
     * @return String containing the return value of the command
     */
    std::string command(const std::string& name, const std::string& arg);

  protected:
    /**
     * @brief Command dispatcher for this device
     *
     * Allows to register commands and calls to be routed to child class member functions. This member is protected and
     * derived classes have direct access to it in order to register their own commands.
     */
    caribou::Dispatcher _dispatcher;

  private:
    /**
     * @brief Private static status flag if devices are managed
     *
     * This is used by the caribou::pearyDevice class to check for other running devices
     */
    static bool managedDevice;

    // Make the device manager a friend class to allow toggling the caribouDevice::managedDevice flag
    friend class caribouDeviceMgr;
  }; // class caribouDevice

} // namespace caribou

#endif /* CARIBOU_API_H */
