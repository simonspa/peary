/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_API_H
#define CARIBOU_API_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

namespace caribou {

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
    caribouDevice();

    /** Default destructor for Caribou devices
     */
    ~caribouDevice() {};
    
    /** Initializer function for Caribou devices
     */
    init() = 0;

    /** Turn on the power supply for the attached device
     */
    powerOn() = 0;

    /** Turn off power for the attached device
     */
    powerOff() = 0;

    /** Start the data acquisition
     */
    daqStart() = 0;

    /** Stop the data acquisition
     */
    daqStop() = 0;

  private:
    /** Instance of the Caribou hardware abstraction layer library
     *
     *  All register and hardware access should go through this interface.
     */
    caribou::caribouHAL hal;
    
  }; //class caribouDevice

} //namespace caribou

#endif /* CARIBOU_API_H */
