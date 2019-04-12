/**
 * Caribou Device Manager class header
 */

#ifndef CARIBOU_DEVMGR_H
#define CARIBOU_DEVMGR_H

#include "Device.hpp"

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
  class DeviceMgr {

  public:
    /** Default constructor
     */
    DeviceMgr();

    /** Destructor
     */
    ~DeviceMgr();

    /** Method to get the Device instance.
     */
    Device* getDevice(size_t id);

    /** Method returning the full list of Devices
     */
    std::vector<Device*> getDevices();

    /** Method to add a new Device instance.
     *  The input parameter is the library name of the device to
     *  be instanciated, the return value is the device ID
     *  assigned by the device manager
     */
    size_t addDevice(std::string name, const caribou::Configuration config);

    /** Method to remove all managed devices and close them by deleting the pointer
     */
    void clearDevices();

  private:
    /** Map of the device library name and the actual pointer to the loaded library
     */
    std::map<std::string, void*> _deviceLibraries;

    /** List of instanciated devices, the position in the device vector represents
     *  their device ID
     */
    std::vector<Device*> _deviceList;

  }; // class DeviceMgr

} // namespace caribou

#endif /* CARIBOU_DEVMGR_H */
