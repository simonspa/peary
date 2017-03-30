/** Caribou datatypes to access regulators and sources
 */

#ifndef CARIBOU_DATATYPES_H
#define CARIBOU_DATATYPES_H

#include <tuple>

/** Component Configuration Base class
 */
class component_t {
public:
  component_t(std::string name) : _name(name) {};
  virtual ~component_t() {};
  std::string name() const {return _name; };
private:
  std::string _name;
};

/** Component Configuration class for components using a DAC output
 */
class component_dac_t : public component_t {
public:
  component_dac_t(std::string name, uint8_t dacaddr, uint8_t dacout) :
    component_t(name),
    _dacaddress(dacaddr),
    _dacoutput(dacout)
  {};
  virtual ~component_dac_t() {};

  uint8_t dacaddress() const { return _dacaddress; };
  uint8_t dacoutput() const { return _dacoutput; };

private:
  uint8_t _dacaddress;
  uint8_t _dacoutput;
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
  VOLTAGE_REGULATOR_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t pwrswitch, uint8_t pwrmon) :
    component_dac_t(name,dacaddr,dacout),
    _powerswitch(pwrswitch),
    _powermonitor(pwrmon)
  {};
  ~VOLTAGE_REGULATOR_T() {};

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
  CURRENT_SOURCE_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t polswitch) :
    component_dac_t(name,dacaddr,dacout),
    _polswitch(polswitch)
  {};
  ~CURRENT_SOURCE_T() {};

  uint8_t polswitch() const { return _polswitch; };
  
private:
  uint8_t _polswitch;
};

/** Slow ADC Channel Configuration
 *
 *  The parameters hold (in this order):
 *  - the channel number
 */
typedef uint8_t SLOW_ADC_CHANNEL_T;

/**  Bias Voltage Regulator Configuration
 *  
 *  - the name of the bias voltage
 *  - the DAC address
 *  - the corresponding DAC output pin
 */
typedef component_dac_t BIAS_REGULATOR_T;

/**  Injection Bias Voltage Regulator Configuration
 *  
 *  - the name of the injection bias
 *  - the DAC address
 *  - the corresponding DAC output pin
 *  - FIXME: INJ_CTRL_X signals from FPGA!
 */
typedef std::tuple<std::string, uint8_t, uint8_t> INJBIAS_REGULATOR_T;

// FIXME
// MISSING
// fast ADC: ADC_IN_XY


#endif /* CARIBOU_DATATYPES_H */
