/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_API_H
#define CARIBOU_API_H

#include "configuration.h"

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
    caribouDevice(caribou::Configuration config);

    /** Default destructor for Caribou devices
     */
    virtual ~caribouDevice();

    /** Return the software version string for reference
     */
    std::string getVersion();

    /** Return the device identifier of the firmware currently loaded
     */
    uint8_t getDeviceID();

    /** Return the human-readable device name of the firmware currently loaded
     */
    std::string getDeviceName();

    /** Initializer function for Caribou devices
     */
    virtual void init() = 0;

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

  protected:
    /** Instance of the Caribou hardware abstraction layer library
     *
     *  All register and hardware access should go through this interface.
     */
    caribou::caribouHAL * _hal;

    /** Device configuration object
     */
    caribou::Configuration _config;
    
  }; //class caribouDevice

} //namespace caribou

#endif /* CARIBOU_API_H */
