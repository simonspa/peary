/** Caribou bare device example
 *
 * Use this class as a starting point for a device that does not rely on
 * an underlying hardware abstraction layer.
 */

#ifndef BARE_EXAMPLE_DEVICE_H
#define BARE_EXAMPLE_DEVICE_H

#include "device.hpp"

namespace caribou {
  /** Bare Example Device class definition
   *
   * This class directly implements all purely virtual functions of
   * `caribou::caribouDevice` without a hardware abstration layer.
   * Most functions are noops. Applications can then control this
   * device via the Caribou device class interface by using the device
   * manager to instanciate the device object.
   */
  class BareExampleDevice final : public caribouDevice {
  public:
    BareExampleDevice(caribou::Configuration config);
    ~BareExampleDevice();

    std::string getFirmwareVersion();
    std::string getName();

    // Controll the device
    void reset(){};
    void configure(){};
    void powerOn();
    void powerOff();
    void daqStart();
    void daqStop();
    // Retrieve data
    std::vector<uint32_t> getRawData();
    pearydata getData();
    // Configure the device
    void setRegister(std::string, uint32_t){};
    uint32_t getRegister(std::string) { return 0u; };
    std::vector<std::pair<std::string, uint32_t>> getRegisters();
    std::vector<uint64_t> timestampsPatternGenerator();
    // Voltage regulators and current sources
    void setVoltage(std::string, double, double){};
    void setBias(std::string, double){};
    void setInjectionBias(std::string, double){};
    void switchOn(std::string){};
    void switchOff(std::string){};
    void setCurrent(std::string, int, bool){};
    double getVoltage(std::string) { return 0.0; };
    double getCurrent(std::string) { return 0.0; };
    double getPower(std::string) { return 0.0; };
    // Slow ADCs
    double getADC(std::string) { return 0.0; };
    double getADC(uint8_t) { return 0.0; };

  private:
    int32_t frobicate(int32_t a);
    std::string unfrobicate(int32_t b);
  };

} // namespace caribou

#endif /* DEVICE_EXAMPLE_H */
