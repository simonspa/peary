/**
 * Caribou Auxiliary Device header
 */

#ifndef CARIBOU_DEVICE_AUXILIARY_H
#define CARIBOU_DEVICE_AUXILIARY_H

#include "Device.hpp"
#include "configuration.hpp"
#include "interface.hpp"
#include "InterfaceManager.hpp"

#include <string>
#include <vector>

namespace caribou {

  /** Caribou Auxiliary Device class definition
   *
   */
  template <typename T> class AuxiliaryDevice : public Device {

  public:
    /** Default constructor for Caribou devices
     *
     */
    AuxiliaryDevice(const caribou::Configuration config, std::string devpath, uint32_t devaddr = 0);

    /** Default destructor for Caribou devices
     */
    virtual ~AuxiliaryDevice();

    /** Return the human-readable device name
     */
    virtual std::string getName() { return getType(); };

    /** Return the human-readable device type
     */
    std::string getType();

    virtual std::string getFirmwareVersion() { return std::string(); };

    virtual std::vector<uint32_t> getRawData() { return std::vector<uint32_t>(); };

    virtual void powerOn(){};
    virtual void powerOff(){};

    virtual void daqStart(){};
    virtual void daqStop(){};
    virtual void setRegister(std::string, uint32_t){};
    virtual uint32_t getRegister(std::string) { return uint32_t(); };
    virtual std::vector<std::pair<std::string, uint32_t>> getRegisters() {
      return std::vector<std::pair<std::string, uint32_t>>();
    };
    virtual std::vector<std::string> listRegisters() { return std::vector<std::string>(); }
    virtual std::vector<std::pair<std::string, std::string>> listComponents() {
      return std::vector<std::pair<std::string, std::string>>();
    };

    virtual void setVoltage(std::string, double, double){};

    virtual void switchOn(std::string){};
    virtual void switchOff(std::string){};
    virtual void setCurrent(std::string, int, bool){};

    virtual double getVoltage(std::string) { return double(); };
    virtual double getCurrent(std::string) { return double(); };
    virtual double getPower(std::string) { return double(); };
    virtual double getADC(std::string) { return double(); };
    virtual double getADC(uint8_t) { return double(); };

    virtual void configure(){};

    // Controlling the device
    virtual void set(std::string, uint32_t){};
    virtual double get(std::string) { return double(); };

    /** Sending reset signal to the device
     */
    virtual void reset(){};

  protected:
    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename T::data_type send(const typename T::data_type& data);

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::pair<typename T::reg_type, typename T::data_type>
    send(const std::pair<typename T::reg_type, typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const typename T::reg_type& reg, const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<std::pair<typename T::reg_type, typename T::data_type>>
    send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data);

    // Read data from a device which does not contain internal register
    std::vector<typename T::data_type> receive(const unsigned int length = 1);

    // Read data from a device containing internal registers
    std::vector<typename T::data_type> receive(const typename T::reg_type reg, const unsigned int length = 1);

  private:
    /** Device interface address
     */
    std::string _devpath;
    uint32_t _devaddress;

    /** Device configuration object
     */
    caribou::Configuration _config;
  }; // class AuxiliaryDevice

} // namespace caribou

#include "AuxiliaryDevice.tcc"

#endif /* CARIBOU_DEVICE_AUXILIARY_H */
