/** Example caribou device
 *
 * Use this class as a starting point for a device that does not rely on
 * an underlying hardware abstraction layer.
 */

#ifndef EXAMPLE_CARIBOU_DEVICE_H
#define EXAMPLE_CARIBOU_DEVICE_H

#include "device.hpp"

namespace caribou {
  /** Example caribou device class definition
   *
   * This class directly implements all purely virtual functions of
   * `caribou::Device` without a hardware abstration layer.
   * Most functions are noops. Applications can then control this
   * device via the Caribou device class interface by using the device
   * manager to instanciate the device object.
   */
  class ExampleCaribouDevice final : public Device {
  public:
    ExampleCaribouDevice(caribou::Configuration config);
    ~ExampleCaribouDevice();

    std::string getFirmwareVersion();
    std::string getType();
    std::string getName() { return getType(); }

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
    std::vector<std::string> listRegisters() { return std::vector<std::string>(); }
    std::vector<std::pair<std::string, std::string>> listComponents() {
      return std::vector<std::pair<std::string, std::string>>();
    }
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

#endif /* EXAMPLE_CARIBOU_DEVICE_H */
