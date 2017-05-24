/** Caribou datatypes to access regulators and sources
 */

#ifndef CARIBOU_DATATYPES_H
#define CARIBOU_DATATYPES_H

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <strings.h>
#include <tuple>

namespace caribou {

  /** Basic pixel class
   *
   *  Storage element for pixel configurations and pixel data.
   *  To be implemented by the individual device classes, deriving
   *  from this common base class.
   */
  class pixel {
  public:
    pixel(){};
    virtual ~pixel(){};

    /** Overloaded ostream operator for printing of pixel data
     */
    friend std::ostream& operator<<(std::ostream& out, const pixel& px) {
      px.print(out);
      return out;
    }

  protected:
    virtual void print(std::ostream&) const {};
  };

  /** Data returned by the peary device interface
   *
   *  Depending on the detector type operated, this can either be one frame
   *  read from the device, or one triggered event. The caribou::pixel pointer
   *  can be any type of data deriving from the pixel base class
   */
  typedef std::map<std::pair<uint16_t, uint16_t>, std::unique_ptr<pixel>> pearydata;

  /** class to store a register configuration
   *
   *  @param address Address of the register in question
   *  @param mask    Mask identifying which bits belong to the register.
   *                 This allows to set registers which only occupy a
   *                 fraction of the full-size register to be written
   *  @param special Flag this register as "special", which forbids reading
   */
  template <typename REG_T = uint8_t, typename MASK_T = uint8_t> class register_t {
  public:
    // If no address is given, also set the mask to zero:
    register_t() : _address(0), _mask(0), _value(0), _special(false){};
    // If no mask is given, default to accessing the full register:
    register_t(REG_T address) : _address(address), _mask(std::numeric_limits<MASK_T>::max()), _value(0), _special(false){};
    register_t(REG_T address, MASK_T mask) : _address(address), _mask(mask), _value(0), _special(false){};
    register_t(REG_T address, MASK_T mask, bool special) : _address(address), _mask(mask), _value(0), _special(special){};
    register_t(REG_T address, MASK_T mask, MASK_T value, bool special = false)
        : _address(address), _mask(mask), _value(value), _special(false){};

    REG_T address() const { return _address; };
    MASK_T mask() const { return _mask; };
    MASK_T value() const { return _value; };
    bool special() const { return _special; };

    MASK_T shift() const {
      if(_mask > 0)
        return (ffs(_mask) - 1);
      else
        return 0;
    };

    template <typename T1, typename T2> friend std::ostream& operator<<(std::ostream& os, const register_t<T1, T2>& rg);

  private:
    REG_T _address;
    MASK_T _mask;
    MASK_T _value;
    bool _special;
  };

  template <typename T1, typename T2> std::ostream& operator<<(std::ostream& os, const caribou::register_t<T1, T2>& rg) {
    os << to_hex_string(rg._address) << " (" << to_bit_string(rg._mask) << ")";
    return os;
  }

  /** Component Configuration Base class
   */
  class component_t {
  public:
    component_t(std::string name) : _name(name){};
    virtual ~component_t(){};
    std::string name() const { return _name; };

  private:
    std::string _name;
  };

  /** Component Configuration class for components using a DAC output
   */
  class component_dac_t : public component_t {
  public:
    component_dac_t(std::string name, uint8_t dacaddr, uint8_t dacout)
        : component_t(name), _dacaddress(dacaddr), _dacoutput(dacout){};
    virtual ~component_dac_t(){};

    uint8_t dacaddress() const { return _dacaddress; };
    uint8_t dacoutput() const { return _dacoutput; };

  private:
    uint8_t _dacaddress;
    uint8_t _dacoutput;
  };

  /** DC/DC converter Configuration
   *
   *  The parameters hold:
   *  - the name of the power output
   *  - the DAC address
   *  - the corresponding DAC output pin
   */
  class DCDC_CONVERTER_T : public component_dac_t {
  public:
    DCDC_CONVERTER_T(std::string name, uint8_t dacaddr, uint8_t dacout) : component_dac_t(name, dacaddr, dacout){};
    ~DCDC_CONVERTER_T(){};
  };

  /** Voltage Regulator Configuration
   *
   *  The parameters hold:
   *  - the name of the power output
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - the output pin of the power switch
   *  - the address of the current/power monitor
   */
  class VOLTAGE_REGULATOR_T : public component_dac_t {
  public:
    VOLTAGE_REGULATOR_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t pwrswitch, uint8_t pwrmon)
        : component_dac_t(name, dacaddr, dacout), _powerswitch(pwrswitch), _powermonitor(pwrmon){};
    ~VOLTAGE_REGULATOR_T(){};

    uint8_t pwrswitch() const { return _powerswitch; };
    uint8_t pwrmonitor() const { return _powermonitor; };

  private:
    uint8_t _powerswitch;
    uint8_t _powermonitor;
  };

  /** Current Source Configuration
   *
   *  The parameters hold:
   *  - the name of the current source
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - the output pin of the polarity switch
   */
  class CURRENT_SOURCE_T : public component_dac_t {
  public:
    CURRENT_SOURCE_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t polswitch)
        : component_dac_t(name, dacaddr, dacout), _polswitch(polswitch){};
    ~CURRENT_SOURCE_T(){};

    uint8_t polswitch() const { return _polswitch; };

  private:
    uint8_t _polswitch;
  };

  /** Slow ADC Channel Configuration
   *
   *  The parameters hold (in this order):
   *  - the channel number
   */
  class SLOW_ADC_CHANNEL_T : public component_t {
  public:
    SLOW_ADC_CHANNEL_T(std::string name, uint8_t channel, uint8_t address)
        : component_t(name), _channel(channel), _address(address){};
    virtual ~SLOW_ADC_CHANNEL_T(){};

    uint8_t channel() const { return _channel; };
    uint8_t address() const { return _address; };

  private:
    uint8_t _channel;
    uint8_t _address;
  };

  /**  Bias Voltage Regulator Configuration
   *
   *  - the name of the bias voltage
   *  - the DAC address
   *  - the corresponding DAC output pin
   */
  // FIXME cannot do this, otherwise casting is ambiguous for the two!
  typedef component_dac_t BIAS_REGULATOR_T;

  /**  Injection Bias Voltage Regulator Configuration
   *
   *  - the name of the injection bias
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - FIXME: INJ_CTRL_X signals from FPGA!
   */
  typedef component_dac_t INJBIAS_REGULATOR_T;

  // FIXME
  // MISSING
  // fast ADC: ADC_IN_XY

} // namespace caribou

#endif /* CARIBOU_DATATYPES_H */
