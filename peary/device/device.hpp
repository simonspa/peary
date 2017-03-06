/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_API_H
#define CARIBOU_API_H

#include "configuration.hpp"
#include "dictionary.hpp"
#include "constants.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace caribou {

  /** Forward declaration of the hardware abstraction layer, not including the header file!
   */
  class caribouHAL;

  /** Caribou Device class definition
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
    caribouDevice(const caribou::Configuration config);

    /** Default destructor for Caribou devices
     */
    virtual ~caribouDevice();

    /** Return the software version string for reference
     */
    std::string getVersion();

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

    /** Report power status
     *  Method should use logINFO as an output
     */
    virtual void powerStatusLog() = 0;

    /** Function to reconfigure the Caribou device
     */
    // Previously "loadConfig"?
    //virtual void reconfigure() {};
    

    
    // Controlling the device
    
    // How to define registers? std::string and internal conversion?
    // Overload to accept vector?
    //virtual void setRegister(std::string name, uint32_t value) {};

    /** Sending reset signal to the device
     */
    //virtual void reset() = 0;

    // Setting the acquisition clock/device clock?
    // Could be either the supplied clock from DAQ or internal clock divider...
    //virtual void setClockFrequency();

    
    // Programming the pixel matrix

    /** Configure the pixel matrix
     */
    // Provide functions both for the full matrix and single pixels?
    // Sometimes, pixel configs will have to be cached by child classes since
    // only programming of full matrix is supported by device...
    //virtual void configureMatrix() = 0;
    //virtual void configurePixel() = 0;

    
    // Voltage regulators
    
    // To set supply voltages, same question as above: how to define voltage names?
    // Separate functions to set target voltage and activate?
    // Purely virtual?
    // Do they need to be virtual? Maybe implement in baseclass (always same for CaR)
    // and only look up correct regulator according to name from child class dictionary?
    void voltageSet(std::string name, double voltage);
    void voltageOn(std::string name);
    void voltageOff(std::string name);
    // Also add disableVoltage() or rather provide boolean parameter for on/off?
    //virtual double measureVoltage();
    //virtual double measureCurrent();

    //virtual double getTemperature();
    //virtual double getADC();
    
    // CaR CMOS signals
    // void enableSignal();
    // void disableSignal();

    
    // Retrieving data

    // Two types:
    //  * trigger based: "events" are returned
    //  * shutter based: "frames" are returned
    // Both contain pixel(s), timestamp(s)
    //virtual std::vector<caribou::event> getData();
    // If no data available, throw caribou::NoDataAvailable exception instead of returning empty vector!
    // Otherwise synchronization of event-based detectors impossible


  protected:
    /** Instance of the Caribou hardware abstraction layer library
     *
     *  All register and hardware access should go through this interface.
     */
    caribou::caribouHAL * _hal;

    /** Return the required communication interface
     */
    virtual IFACE interface() = 0;

    /** Initializer function for Caribou devices
     */
    void initialize(std::string devpath, caribou::dictionary<uint8_t> periphery);

    /** Device configuration object
     */
    caribou::Configuration _config;

    /** Static periphery dictionary for all devices to access CaR components:
     */
    static caribou::dictionary<uint8_t> _periphery;

  }; //class caribouDevice

} //namespace caribou

#endif /* CARIBOU_API_H */
