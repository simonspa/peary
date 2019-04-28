/**
 * Caribou Dso9254a Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_DSO9254A_H
#define DEVICE_DSO9254A_H

#include <vector>
#include "AuxiliaryDevice.hpp"
#include "Device.hpp"
#include "constants.hpp"

#define DEFAULT_DEVICEPATH "127.0.0.1:5000"

namespace caribou {

  /** Dso9254a Device class definition
   *
   *  This class implements all purely virtual functions of caribou::Device.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class DSO9254ADevice : public AuxiliaryDevice<iface_ipsocket> {

  public:
    /** Device constructor
     */
    DSO9254ADevice(const caribou::Configuration config);
    ~DSO9254ADevice();

    void send(std::string command);
    std::string query(const std::string query);

    pearydata getData();

    int waitForTrigger();

  private:
    caribou::Configuration _config;
    std::string _devpath;
  };

} // namespace caribou

#endif /* DEVICE_DSO9254A_H */
