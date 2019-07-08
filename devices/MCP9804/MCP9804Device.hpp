/**
 * Caribou MCP9804 Device implementation
 */

#ifndef DEVICE_MCP9804_H
#define DEVICE_MCP9804_H

#include <vector>

#include "device/AuxiliaryDevice.hpp"
#include "interfaces/I2C/i2c.hpp"

namespace caribou {

  /** a4529osD Temperature Sensor Device class definition
   */
  class MCP9804Device : public AuxiliaryDevice<iface_i2c> {

  public:
    /** Device constructor
     */
    MCP9804Device(const caribou::Configuration config);
    ~MCP9804Device();

    pearydata getData();
    void getTemperature();

  private:
    double readTemperature();

    caribou::Configuration _config;
    std::string _devpath;
  };

} // namespace caribou

#endif /* DEVICE_MCP9804_H */
