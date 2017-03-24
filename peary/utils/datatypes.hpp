/** Caribou datatypes to access regulators and sources
 */

#ifndef CARIBOU_DATATYPES_H
#define CARIBOU_DATATYPES_H

#include <tuple>

 
/** Voltage Regulator Configuration
 *
 *  The parameters hold (in this order):
 *  - the name of the power output
 *  - the DAC address
 *  - the corresponding DAC output pin
 *  - the output pin of the power switch
 *  - the address of the current/power monitor
 */
typedef std::tuple<std::string, uint8_t, uint8_t, uint8_t, uint8_t> VOLTAGE_REGULATOR_T;

/** Current Source Configuration
 *
 *  The parameters hold (in this order):
 *  - the name of the current source
 *  - the DAC address
 *  - the corresponding DAC output pin
 *  - the output pin of the polarity switch
 */
typedef std::tuple<std::string, uint8_t, uint8_t, uint8_t> CURRENT_SOURCE_T;

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
typedef std::tuple<std::string, uint8_t, uint8_t> BIAS_REGULATOR_T;

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
