/**
 * Caribou Dso9254a Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_DSO9254A_H
#define DEVICE_DSO9254A_H

#include <vector>
#include "auxiliarydevice.hpp"
#include "constants.hpp"
#include "device.hpp"
#include "ipsocket.hpp"

#define DEFAULT_DEVICEPATH "127.0.0.1:5000"

namespace caribou {

  /** Dso9254a Device class definition
   *
   *  This class implements all purely virtual functions of caribou::caribouDevice.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class dso9254a : public auxiliaryDevice<iface_ipsocket> {

  public:
    /** Device constructor
     */
    dso9254a(const caribou::Configuration config);
    ~dso9254a();

    /** Return human-readable name of this device
     */
    std::string getName();

    void send(std::string command);
    std::string query(const std::string query);

    pearydata getData();

    int waitForTrigger();

  private:
    caribou::Configuration _config;
    std::string _devpath;
  };

  extern "C" {
  caribouDevice* generator(const caribou::Configuration);
  }

} // namespace caribou

#endif /* DEVICE_DSO9254A_H */
