/**
 * Caribou Device Manager class header
 */

#ifndef CARIBOU_DEVMGR_H
#define CARIBOU_DEVMGR_H

#include "device.hpp"

#include <stdint.h>
#include <string>
#include <vector>

namespace caribou {

  /** Caribou Device Manager
   *
   *  This class holds is the interface between the individual
   *  devices and the actual setup. It loads the required device
   *  device library at runtime, the device library is identified
   *  by the device name.
   */
  class caribouDeviceMgr {

  public:
    /** Default constructor
     */
    caribouDeviceMgr();

    /** Destructor
     */
    ~caribouDeviceMgr();

    /** Method to get the caribouDevice instance.
     */
    caribouDevice* getDevice(size_t id);

    /** Method returning the full list of caribouDevices
     */
    std::vector<caribouDevice*> getDevices();

    /** Method to add a new caribouDevice instance.
     *  The input parameter is the library name of the device to
     *  be instanciated, the return value is the device ID
     *  assigned by the device manager
     */
    size_t addDevice(std::string name, const caribou::Configuration config);

  private:
    /** Map of the device library name and the actual pointer to the loaded library
     */
    std::map<std::string, void*> _deviceLibraries;

    /** List of instanciated devices, the position in the device vector represents
     *  their device ID
     */
    std::vector<caribouDevice*> _deviceList;

  }; // class caribouDeviceMgr

} // namespace caribou

#endif /* CARIBOU_DEVMGR_H */
