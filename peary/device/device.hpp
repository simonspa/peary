/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_API_H
#define CARIBOU_API_H

#include "configuration.hpp"
#include "constants.hpp"
#include "datatypes.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace caribou {

  /** Abstract Caribou Device class definition
   *
   *  this is the central device class from which all device implementations inherit.
   *
   *  Some basic functionality is defined via purely virtual member functions which
   *  have to be implemented by every device instance. This enables the possibility
   *  of interfacing the devices independently via the common set of function alls, e.g.,
   *  from a GUI or a commandline interface.
   */
  class caribouDevice {

  public:
    /** Default constructor for Caribou devices
     *
     */
    caribouDevice(const caribou::Configuration);

    /** Default destructor for Caribou devices
     */
    virtual ~caribouDevice(){};

    /** Indicator flag for managed devices
     */
    bool isManaged() { return managedDevice; };

    /** Return the software version string for reference
     */
    std::string getVersion();

    /** Return the firmware version
     */
    virtual std::string getFirmwareVersion() = 0;

    /** Return the human-readable device name
     */
    virtual std::string getName() = 0;

    /** Return the identifier of the firmware currently loaded
     */
    virtual uint8_t getFirmwareID() = 0;

    /** Return the board identifier of the CaR board installed
     */
    virtual uint8_t getCaRBoardID() = 0;

    /** Read the ID from the chip board if available
     *
     *  Some chip boards feature an EPROM which stores a board ID and thus
     *  allows identification of the attached chip board.
     */
    virtual uint16_t getChipID() = 0;

    /** Return the human-readable device name of the firmware currently loaded
     */
    virtual std::string getDeviceName() = 0;

    /** Turn on the power supply for the attached device
     */
    virtual void powerOn() = 0;

    /** Turn off power for the attached device
     */
    virtual void powerOff() = 0;

    /** Start the data acquisition
     */
    virtual void daqStart() = 0;

    /** Stop the data acquisition
     */
    virtual void daqStop() = 0;

    /** Get data methods. Can return raw or decoded data **/
    virtual std::vector<uint32_t> getRawData() = 0;
    virtual pearydata getData() = 0;

    /** Report power status
     *  Method should use logINFO as an output
     */
    virtual void powerStatusLog() = 0;

    /** Explore interface by sending data via all available functions
     *  FIXME for debugging purposes, should be removed later
     */
    virtual void exploreInterface() = 0;

    /** Initialize the device (ex.set the required clock otuputs etc.).
     *  Function to configure the Caribou device by setting all DACs to the values
     *  provided via the initial configuration object
     */
    virtual void configure() = 0;

    // Controlling the device

    /** Set register on the device
     *
     *  The register is identified by its human-readable name, the value is
     *  automatically casted to the register data type (e.g. 8-bit)
     */
    virtual void setRegister(std::string name, uint32_t value) = 0;

    /** Get register from the device
     *
     *  Retrieve content of specified register from the device. The register
     *  is identified by its human-readable name.
     */
    virtual uint32_t getRegister(std::string name) = 0;
    virtual std::vector<std::pair<std::string, uint32_t>> getRegisters() = 0;

    /** Sending reset signal to the device
     */
    // virtual void reset() = 0;

    // Setting the acquisition clock/device clock?
    // Could be either the supplied clock from DAQ or internal clock divider...
    // virtual void setClockFrequency();

    /** Configure the pattern generator
     *
     *
     */
    virtual void configurePatternGenerator(std::string filename) = 0;

    /** Execute the pattern generator
     *
     *  Trigger the execution of the pattern generator once
     */
    virtual void triggerPatternGenerator() = 0;

    /** Configure the pixel matrix
     */
    // Provide functions both for the full matrix and single pixels?
    // Sometimes, pixel configs will have to be cached by child classes since
    // only programming of full matrix is supported by device...
    virtual void configureMatrix(std::string filename) = 0;
    // virtual void configurePixel() = 0;

    // Voltage regulators

    // To set supply voltages, same question as above: how to define voltage names?
    // Separate functions to set target voltage and activate?
    // Purely virtual?
    // Do they need to be virtual? Maybe implement in baseclass (always same for CaR)
    // and only look up correct regulator according to name from child class dictionary?
    virtual void setVoltage(std::string name, double voltage) = 0;
    virtual void setBias(std::string name, double voltage) = 0;
    virtual void setInjectionBias(std::string name, double voltage) = 0;

    virtual void voltageOn(std::string name) = 0;
    virtual void voltageOff(std::string name) = 0;
    virtual void biasOn(std::string name) = 0;
    virtual void biasOff(std::string name) = 0;

    virtual void setCurrent(std::string name, double current) = 0;
    virtual double getVoltage(std::string name) = 0;
    virtual double getCurrent(std::string name) = 0;
    virtual double getPower(std::string name) = 0;

    // virtual double getTemperature();

    /** Read slow-ADC value by name of the input signal as defined by the device
     *
     *  Returns value in SI Volts
     */
    virtual double getADC(std::string name) = 0;

    /** Read slow-ADC value by the input channel number of the ADC device
     *
     *  Returns value in SI Volts
     */
    virtual double getADC(uint8_t channel) = 0;

    // CaR CMOS signals
    // void enableSignal();
    // void disableSignal();

    // Retrieving data

    // Two types:
    //  * trigger based: "events" are returned
    //  * shutter based: "frames" are returned
    // Both contain pixel(s), timestamp(s)
    // virtual std::vector<caribou::event> getData();
    // If no data available, throw caribou::NoDataAvailable exception instead of returning empty vector!
    // Otherwise synchronization of event-based detectors impossible

  private:
    /** Private static status flag if devices are managed
     *
     *  This is used by the caribou::pearyDevice class to check for other running devices
     */
    static bool managedDevice;

    // Make the device manager a friend class to allow toggling the caribouDevice::managedDevice flag
    friend class caribouDeviceMgr;
  }; // class caribouDevice

} // namespace caribou

#endif /* CARIBOU_API_H */
