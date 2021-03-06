/*
 * Si5345 Rev B Configuration Register Export Header File
 *
 * This file represents a series of Silicon Labs Si5345 Rev B
 * register writes that can be performed to load a single configuration
 * on a device. It was created by a Silicon Labs ClockBuilder Pro
 * export tool.
 *
 * Part:		                                       Si5345 Rev B
 * Design ID:                                          CLICpix2
 * Includes Pre/Post Download Control Register Writes: Yes
 * Created By:                                         ClockBuilder Pro v2.15 [2017-05-15]
 * Timestamp:                                          2017-08-10 14:23:08 GMT+02:00
 *
 * A complete design report corresponding to this export is included at the end
 * of this header file.
 *
 */

#ifndef SI5345_REVB_REG_CONFIG_HEADER_FREE
#define SI5345_REVB_REG_CONFIG_HEADER_FREE

#include "utils/datatypes.hpp"
using namespace caribou;

#define SI5345_REVB_REG_CONFIG_NUM_REGS_FREE 386

si5345_revb_register_t const si5345_revb_registers_free[SI5345_REVB_REG_CONFIG_NUM_REGS_FREE] = {

  /* Start configuration preamble */
  {0x0B24, 0xD8},
  {0x0B25, 0x00},
  {0x0540, 0x01},
  /* End configuration preamble */

  /* Delay 300 msec */
  /*    Delay is worst case time for device to complete any calibration */
  /*    that is running due to device state change previous to this script */
  /*    being processed. */

  /* Start configuration registers */
  {0x000B, 0x68},
  {0x0016, 0x02},
  {0x0017, 0x3C},
  {0x0018, 0xFF},
  {0x0019, 0xFF},
  {0x001A, 0xFF},
  {0x002B, 0x02},
  {0x002C, 0x00},
  {0x002D, 0x00},
  {0x002E, 0x00},
  {0x002F, 0x00},
  {0x0030, 0x00},
  {0x0031, 0x00},
  {0x0032, 0x00},
  {0x0033, 0x00},
  {0x0034, 0x00},
  {0x0035, 0x00},
  {0x0036, 0x00},
  {0x0037, 0x00},
  {0x0038, 0x00},
  {0x0039, 0x00},
  {0x003A, 0x00},
  {0x003B, 0x00},
  {0x003C, 0x00},
  {0x003D, 0x00},
  {0x003F, 0x00},
  {0x0040, 0x04},
  {0x0041, 0x00},
  {0x0042, 0x00},
  {0x0043, 0x00},
  {0x0044, 0x00},
  {0x0045, 0x0C},
  {0x0046, 0x00},
  {0x0047, 0x00},
  {0x0048, 0x00},
  {0x0049, 0x00},
  {0x004A, 0x00},
  {0x004B, 0x00},
  {0x004C, 0x00},
  {0x004D, 0x00},
  {0x004E, 0x00},
  {0x004F, 0x00},
  {0x0051, 0x00},
  {0x0052, 0x00},
  {0x0053, 0x00},
  {0x0054, 0x00},
  {0x0055, 0x00},
  {0x0056, 0x00},
  {0x0057, 0x00},
  {0x0058, 0x00},
  {0x0059, 0x00},
  {0x005A, 0x00},
  {0x005B, 0x00},
  {0x005C, 0x00},
  {0x005D, 0x00},
  {0x005E, 0x00},
  {0x005F, 0x00},
  {0x0060, 0x00},
  {0x0061, 0x00},
  {0x0062, 0x00},
  {0x0063, 0x00},
  {0x0064, 0x00},
  {0x0065, 0x00},
  {0x0066, 0x00},
  {0x0067, 0x00},
  {0x0068, 0x00},
  {0x0069, 0x00},
  {0x0092, 0x00},
  {0x0093, 0x00},
  {0x0095, 0x00},
  {0x0096, 0x00},
  {0x0098, 0x00},
  {0x009A, 0x00},
  {0x009B, 0x00},
  {0x009D, 0x00},
  {0x009E, 0x00},
  {0x00A0, 0x00},
  {0x00A2, 0x00},
  {0x00A8, 0x00},
  {0x00A9, 0x00},
  {0x00AA, 0x00},
  {0x00AB, 0x00},
  {0x00AC, 0x00},
  {0x0102, 0x01},
  {0x0108, 0x01},
  {0x0109, 0x09},
  {0x010A, 0x3B},
  {0x010B, 0x00},
  {0x010D, 0x01},
  {0x010E, 0x09},
  {0x010F, 0x3B},
  {0x0110, 0x00},
  {0x0112, 0x06},
  {0x0113, 0x09},
  {0x0114, 0x3B},
  {0x0115, 0x00},
  {0x0117, 0x01},
  {0x0118, 0x09},
  {0x0119, 0x3B},
  {0x011A, 0x00},
  {0x011C, 0x06},
  {0x011D, 0x09},
  {0x011E, 0x3B},
  {0x011F, 0x01},
  {0x0121, 0x01},
  {0x0122, 0x09},
  {0x0123, 0x3B},
  {0x0124, 0x00},
  {0x0126, 0x06},
  {0x0127, 0x09},
  {0x0128, 0x3B},
  {0x0129, 0x01},
  {0x012B, 0x01},
  {0x012C, 0x09},
  {0x012D, 0x3B},
  {0x012E, 0x00},
  {0x0130, 0x06},
  {0x0131, 0x09},
  {0x0132, 0x3B},
  {0x0133, 0x00},
  {0x013A, 0x01},
  {0x013B, 0x09},
  {0x013C, 0x3B},
  {0x013D, 0x00},
  {0x013F, 0x00},
  {0x0140, 0x00},
  {0x0141, 0x40},
  {0x0142, 0xFF},
  {0x0202, 0x00},
  {0x0203, 0x00},
  {0x0204, 0x00},
  {0x0205, 0x00},
  {0x0206, 0x00},
  {0x0208, 0x00},
  {0x0209, 0x00},
  {0x020A, 0x00},
  {0x020B, 0x00},
  {0x020C, 0x00},
  {0x020D, 0x00},
  {0x020E, 0x00},
  {0x020F, 0x00},
  {0x0210, 0x00},
  {0x0211, 0x00},
  {0x0212, 0x00},
  {0x0213, 0x00},
  {0x0214, 0x00},
  {0x0215, 0x00},
  {0x0216, 0x00},
  {0x0217, 0x00},
  {0x0218, 0x00},
  {0x0219, 0x00},
  {0x021A, 0x00},
  {0x021B, 0x00},
  {0x021C, 0x00},
  {0x021D, 0x00},
  {0x021E, 0x00},
  {0x021F, 0x00},
  {0x0220, 0x00},
  {0x0221, 0x00},
  {0x0222, 0x00},
  {0x0223, 0x00},
  {0x0224, 0x00},
  {0x0225, 0x00},
  {0x0226, 0x00},
  {0x0227, 0x00},
  {0x0228, 0x00},
  {0x0229, 0x00},
  {0x022A, 0x00},
  {0x022B, 0x00},
  {0x022C, 0x00},
  {0x022D, 0x00},
  {0x022E, 0x00},
  {0x022F, 0x00},
  {0x0231, 0x01},
  {0x0232, 0x01},
  {0x0233, 0x01},
  {0x0234, 0x01},
  {0x0235, 0x00},
  {0x0236, 0x00},
  {0x0237, 0x00},
  {0x0238, 0x80},
  {0x0239, 0xD4},
  {0x023A, 0x00},
  {0x023B, 0x00},
  {0x023C, 0x00},
  {0x023D, 0x00},
  {0x023E, 0xC0},
  {0x024A, 0x00},
  {0x024B, 0x00},
  {0x024C, 0x00},
  {0x024D, 0x00},
  {0x024E, 0x00},
  {0x024F, 0x00},
  {0x0250, 0x00},
  {0x0251, 0x00},
  {0x0252, 0x00},
  {0x0253, 0x00},
  {0x0254, 0x00},
  {0x0255, 0x00},
  {0x0256, 0x00},
  {0x0257, 0x00},
  {0x0258, 0x00},
  {0x0259, 0x00},
  {0x025A, 0x00},
  {0x025B, 0x00},
  {0x025C, 0x00},
  {0x025D, 0x00},
  {0x025E, 0x00},
  {0x025F, 0x00},
  {0x0260, 0x00},
  {0x0261, 0x00},
  {0x0262, 0x00},
  {0x0263, 0x00},
  {0x0264, 0x00},
  {0x0268, 0x00},
  {0x0269, 0x00},
  {0x026A, 0x00},
  {0x026B, 0x43},
  {0x026C, 0x4C},
  {0x026D, 0x49},
  {0x026E, 0x43},
  {0x026F, 0x70},
  {0x0270, 0x69},
  {0x0271, 0x78},
  {0x0272, 0x32},
  {0x0302, 0x00},
  {0x0303, 0x00},
  {0x0304, 0x00},
  {0x0305, 0x00},
  {0x0306, 0x22},
  {0x0307, 0x00},
  {0x0308, 0x00},
  {0x0309, 0x00},
  {0x030A, 0x00},
  {0x030B, 0x80},
  {0x030C, 0x00},
  {0x030D, 0x00},
  {0x030E, 0x00},
  {0x030F, 0x00},
  {0x0310, 0xA0},
  {0x0311, 0x0A},
  {0x0312, 0x00},
  {0x0313, 0x00},
  {0x0314, 0x00},
  {0x0315, 0x00},
  {0x0316, 0x80},
  {0x0317, 0x00},
  {0x0318, 0x00},
  {0x0319, 0x00},
  {0x031A, 0x00},
  {0x031B, 0x00},
  {0x031C, 0x00},
  {0x031D, 0x00},
  {0x031E, 0x00},
  {0x031F, 0x00},
  {0x0320, 0x00},
  {0x0321, 0x00},
  {0x0322, 0x00},
  {0x0323, 0x00},
  {0x0324, 0x00},
  {0x0325, 0x00},
  {0x0326, 0x00},
  {0x0327, 0x00},
  {0x0328, 0x00},
  {0x0329, 0x00},
  {0x032A, 0x00},
  {0x032B, 0x00},
  {0x032C, 0x00},
  {0x032D, 0x00},
  {0x032E, 0x00},
  {0x032F, 0x00},
  {0x0330, 0x00},
  {0x0331, 0x00},
  {0x0332, 0x00},
  {0x0333, 0x00},
  {0x0334, 0x00},
  {0x0335, 0x00},
  {0x0336, 0x00},
  {0x0337, 0x00},
  {0x0338, 0x00},
  {0x0339, 0x1F},
  {0x033B, 0x00},
  {0x033C, 0x00},
  {0x033D, 0x00},
  {0x033E, 0x00},
  {0x033F, 0x00},
  {0x0340, 0x00},
  {0x0341, 0x00},
  {0x0342, 0x00},
  {0x0343, 0x00},
  {0x0344, 0x00},
  {0x0345, 0x00},
  {0x0346, 0x00},
  {0x0347, 0x00},
  {0x0348, 0x00},
  {0x0349, 0x00},
  {0x034A, 0x00},
  {0x034B, 0x00},
  {0x034C, 0x00},
  {0x034D, 0x00},
  {0x034E, 0x00},
  {0x034F, 0x00},
  {0x0350, 0x00},
  {0x0351, 0x00},
  {0x0352, 0x00},
  {0x0353, 0x00},
  {0x0354, 0x00},
  {0x0355, 0x00},
  {0x0356, 0x00},
  {0x0357, 0x00},
  {0x0358, 0x00},
  {0x0359, 0x00},
  {0x035A, 0x00},
  {0x035B, 0x00},
  {0x035C, 0x00},
  {0x035D, 0x00},
  {0x035E, 0x00},
  {0x035F, 0x00},
  {0x0360, 0x00},
  {0x0361, 0x00},
  {0x0362, 0x00},
  {0x0487, 0x00},
  {0x0502, 0x01},
  {0x0508, 0x00},
  {0x0509, 0x00},
  {0x050A, 0x00},
  {0x050B, 0x00},
  {0x050C, 0x00},
  {0x050D, 0x00},
  {0x050E, 0x00},
  {0x050F, 0x00},
  {0x0510, 0x00},
  {0x0511, 0x00},
  {0x0512, 0x00},
  {0x0513, 0x00},
  {0x0515, 0x00},
  {0x0516, 0x00},
  {0x0517, 0x00},
  {0x0518, 0x00},
  {0x0519, 0x00},
  {0x051A, 0x00},
  {0x051B, 0x00},
  {0x051C, 0x00},
  {0x051D, 0x00},
  {0x051E, 0x00},
  {0x051F, 0x00},
  {0x0521, 0x21},
  {0x052A, 0x05},
  {0x052B, 0x00},
  {0x052C, 0x0F},
  {0x052D, 0x03},
  {0x052E, 0x00},
  {0x052F, 0x00},
  {0x0531, 0x00},
  {0x0532, 0x00},
  {0x0533, 0x04},
  {0x0534, 0x00},
  {0x0535, 0x01},
  {0x0536, 0x0C},
  {0x0537, 0x00},
  {0x0538, 0x00},
  {0x0539, 0x00},
  {0x0802, 0x35},
  {0x0803, 0x05},
  {0x0804, 0x01},
  {0x090E, 0x02},
  {0x0943, 0x01},
  {0x0949, 0x00},
  {0x094A, 0x00},
  {0x0A02, 0x00},
  {0x0A03, 0x03},
  {0x0A04, 0x01},
  {0x0A05, 0x03},
  {0x0B44, 0x0F},
  {0x0B46, 0x00},
  {0x0B47, 0x00},
  {0x0B48, 0x0F},
  {0x0B4A, 0x1C},
  /* End configuration registers */

  /* Start configuration postamble */
  {0x0514, 0x01},
  {0x001C, 0x01},
  {0x0540, 0x00},
  {0x0B24, 0xDB},
  {0x0B25, 0x02},
  /* End configuration postamble */

};

/*
 * Design Report
 *
 * Overview
 * ========
 * Part:         Si5345 Rev B
 * Project File: E:\clic\Caribou\peary\devices\clicpix2\Si5345-RevB-CLICpix2-Project_freeRunningMode.slabtimeproj
 * Design ID:    CLICpix2
 * Created By:   ClockBuilder Pro v2.15 [2017-05-15]
 * Timestamp:    2017-08-10 14:23:08 GMT+02:00
 *
 * Design Rule Check
 * =================
 * Errors:
 * - No errors
 *
 * Warnings:
 * - Revision B is not recommended for new designs [Marked Ignore]
 *
 * Device Grade
 * ============
 * Maximum Output Frequency: 320 MHz
 * Frequency Synthesis Mode: Fractional
 * Frequency Plan Grade:     B
 * Minimum Base OPN:         Si5345B*
 *
 * Base       Output Clock         Supported Frequency Synthesis Modes
 * OPN Grade  Frequency Range      (Typical Jitter)
 * ---------  -------------------  --------------------------------------------
 * Si5345A    100 Hz to 712.5 MHz  Integer (< 100 fs) and fractional (< 150 fs)
 * Si5345B*   100 Hz to 350 MHz    "
 * Si5345C    100 Hz to 712.5 MHz  Integer only (< 100 fs)
 * Si5345D    100 Hz to 350 MHz    "
 *
 * * Based on your calculated frequency plan, a Si5345B grade device is
 * sufficient for your design. For more in-system configuration flexibility
 * (higher frequencies and/or to enable fractional synthesis), consider
 * selecting device grade Si5345A when specifying an ordering part number (OPN)
 * for your application. See the datasheet Ordering Guide for more information.
 *
 * Design
 * ======
 * Host Interface:
 *    I/O Power Supply: VDDA (3.3V)
 *    SPI Mode: 4-Wire
 *    I2C Address Range: 104d to 107d / 0x68 to 0x6B (selected via A0/A1 pins)
 *
 * XA/XB:
 *    48 MHz (XTAL - Crystal)
 *
 * Inputs:
 *     IN0: Unused
 *     IN1: Unused
 *     IN2: Unused
 *     IN3: Unused
 *
 * Outputs:
 *    OUT0: Unused
 *    OUT1: Unused
 *    OUT2: 100 MHz (Free Run)
 *          Enabled, LVDS 2.5 V
 *    OUT3: Unused
 *    OUT4: 320 MHz (Free Run)
 *          Enabled, LVDS 2.5 V
 *    OUT5: Unused
 *    OUT6: 320 MHz (Free Run)
 *          Enabled, LVDS 2.5 V
 *    OUT7: Unused
 *    OUT8: 100 MHz (Free Run)
 *          Enabled, LVDS 2.5 V
 *    OUT9: Unused
 *
 * Frequency Plan
 * ==============
 * Priority: maximize the number of low jitter outputs
 *
 * Fvco = 13.6 GHz [ 13 + 3/5 GHz ]
 * Fpfd = N/A (DSPLL free run only mode enabled)
 * Fms0 = 200 MHz
 * Fms1 = 640 MHz
 *
 * P dividers:
 *    P0  = Unused
 *    P1  = Unused
 *    P2  = Unused
 *    P3  = Unused
 *    Pxaxb = 1
 *
 * MXAXB = 283.3333333333333333... [ 283 + 1/3 ]
 * M = N/A (DSPLL free run only mode enabled)
 * N dividers:
 *    N0:
 *       Value: 68
 *       Skew:  0.000 s
 *       OUT2: 100 MHz
 *       OUT8: 100 MHz
 *    N1:
 *       Value: 21.25 [ 21 + 1/4 ]
 *       Skew:  0.000 s
 *       OUT4: 320 MHz
 *       OUT6: 320 MHz
 *    N2:
 *       Unused
 *    N3:
 *       Unused
 *    N4:
 *       Unused
 *
 * R dividers:
 *    R0 = Unused
 *    R1 = Unused
 *    R2 = 2
 *    R3 = Unused
 *    R4 = 2
 *    R5 = Unused
 *    R6 = 2
 *    R7 = Unused
 *    R8 = 2
 *    R9 = Unused
 *
 * Nominal Bandwidth:
 *   N/A (DSPLL free run only mode enabled)
 * Fastlock Bandwidth:
 *   N/A (DSPLL free run only mode enabled)
 *
 * Dividers listed above show effective values. These values are translated to register settings by ClockBuilder Pro. For the
 * actual register values, see below. Refer to the Family Reference Manual for information on registers related to frequency
 * plan.
 *
 * Digitally Controlled Oscillator (DCO)
 * =====================================
 * Mode: Register Direct Write
 *
 * N0: DCO Disabled
 *
 * N1: DCO Disabled
 *
 * N2: DCO Disabled
 *
 * N3: DCO Disabled
 *
 * N4: DCO Disabled
 *
 * Revision B Frequency Offset Errata Report
 * =========================================
 * N/A (DSPLL free run only mode enabled)
 *
 * Estimated Power & Junction Temperature
 * ======================================
 * Assumptions:
 *
 * Revision: B
 * VDD:      1.8 V
 * Ta:       70 �C
 * Airflow:  None
 *
 * Total Power: 761 mW, On Chip Power: 737 mW, Tj: 86 �C
 *
 *         Frequency  Format   Voltage   Current     Power
 *         ---------  ------  --------  --------  --------
 * VDD                           1.8 V  117.1 mA    211 mW
 * VDDA                          3.3 V  117.4 mA    387 mW
 * VDDO0      Unused
 * VDDO1      Unused
 * VDDO2     100 MHz  LVDS       2.5 V   15.6 mA     39 mW
 * VDDO3      Unused
 * VDDO4     320 MHz  LVDS       2.5 V   17.0 mA     43 mW
 * VDDO5      Unused
 * VDDO6     320 MHz  LVDS       2.5 V   17.0 mA     43 mW
 * VDDO7      Unused
 * VDDO8     100 MHz  LVDS       2.5 V   15.6 mA     39 mW
 * VDDO9      Unused
 *                                      --------  --------
 *                               Total  299.7 mA    761 mW
 *
 * Note:
 *
 * -Total power includes on- and off-chip power. This is a typical value and estimate only.
 * -Use an EVB for a more exact power measurement
 * -On-chip power excludes power dissipated in external terminations.
 * -Tj is junction temperature. Tj must be less than 125 �C (on Si5345 Revision B) for device to comply with datasheet
 * specifications.
 *
 * Settings
 * ========
 *
 * Location      Setting Name          Decimal Value  Hex Value
 * ------------  --------------------  -------------  ----------------
 * 0x000B[6:0]   I2C_ADDR              104            0x68
 * 0x0016[1]     LOL_ON_HOLD           1              0x1
 * 0x0017[0]     SYSINCAL_INTR_MSK     0              0x0
 * 0x0017[1]     LOSXAXB_INTR_MSK      0              0x0
 * 0x0017[5]     SMB_TMOUT_INTR_MSK    1              0x1
 * 0x0018[3:0]   LOS_INTR_MSK          15             0xF
 * 0x0018[7:4]   OOF_INTR_MSK          15             0xF
 * 0x0019[1]     LOL_INTR_MSK          1              0x1
 * 0x0019[5]     HOLD_INTR_MSK         1              0x1
 * 0x001A[5]     CAL_INTR_MSK          1              0x1
 * 0x002B[3]     SPI_3WIRE             0              0x0
 * 0x002B[5]     AUTO_NDIV_UPDATE      0              0x0
 * 0x002C[3:0]   LOS_EN                0              0x0
 * 0x002C[4]     LOSXAXB_DIS           0              0x0
 * 0x002D[1:0]   LOS0_VAL_TIME         0              0x0
 * 0x002D[3:2]   LOS1_VAL_TIME         0              0x0
 * 0x002D[5:4]   LOS2_VAL_TIME         0              0x0
 * 0x002D[7:6]   LOS3_VAL_TIME         0              0x0
 * 0x002E[15:0]  LOS0_TRG_THR          0              0x0000
 * 0x0030[15:0]  LOS1_TRG_THR          0              0x0000
 * 0x0032[15:0]  LOS2_TRG_THR          0              0x0000
 * 0x0034[15:0]  LOS3_TRG_THR          0              0x0000
 * 0x0036[15:0]  LOS0_CLR_THR          0              0x0000
 * 0x0038[15:0]  LOS1_CLR_THR          0              0x0000
 * 0x003A[15:0]  LOS2_CLR_THR          0              0x0000
 * 0x003C[15:0]  LOS3_CLR_THR          0              0x0000
 * 0x003F[3:0]   OOF_EN                0              0x0
 * 0x003F[7:4]   FAST_OOF_EN           0              0x0
 * 0x0040[2:0]   OOF_REF_SEL           4              0x4
 * 0x0041[4:0]   OOF0_DIV_SEL          0              0x00
 * 0x0042[4:0]   OOF1_DIV_SEL          0              0x00
 * 0x0043[4:0]   OOF2_DIV_SEL          0              0x00
 * 0x0044[4:0]   OOF3_DIV_SEL          0              0x00
 * 0x0045[4:0]   OOFXO_DIV_SEL         12             0x0C
 * 0x0046[7:0]   OOF0_SET_THR          0              0x00
 * 0x0047[7:0]   OOF1_SET_THR          0              0x00
 * 0x0048[7:0]   OOF2_SET_THR          0              0x00
 * 0x0049[7:0]   OOF3_SET_THR          0              0x00
 * 0x004A[7:0]   OOF0_CLR_THR          0              0x00
 * 0x004B[7:0]   OOF1_CLR_THR          0              0x00
 * 0x004C[7:0]   OOF2_CLR_THR          0              0x00
 * 0x004D[7:0]   OOF3_CLR_THR          0              0x00
 * 0x004E[2:0]   OOF0_DETWIN_SEL       0              0x0
 * 0x004E[6:4]   OOF1_DETWIN_SEL       0              0x0
 * 0x004F[2:0]   OOF2_DETWIN_SEL       0              0x0
 * 0x004F[6:4]   OOF3_DETWIN_SEL       0              0x0
 * 0x0051[3:0]   FAST_OOF0_SET_THR     0              0x0
 * 0x0052[3:0]   FAST_OOF1_SET_THR     0              0x0
 * 0x0053[3:0]   FAST_OOF2_SET_THR     0              0x0
 * 0x0054[3:0]   FAST_OOF3_SET_THR     0              0x0
 * 0x0055[3:0]   FAST_OOF0_CLR_THR     0              0x0
 * 0x0056[3:0]   FAST_OOF1_CLR_THR     0              0x0
 * 0x0057[3:0]   FAST_OOF2_CLR_THR     0              0x0
 * 0x0058[3:0]   FAST_OOF3_CLR_THR     0              0x0
 * 0x0059[1:0]   FAST_OOF0_DETWIN_SEL  0              0x0
 * 0x0059[3:2]   FAST_OOF1_DETWIN_SEL  0              0x0
 * 0x0059[5:4]   FAST_OOF2_DETWIN_SEL  0              0x0
 * 0x0059[7:6]   FAST_OOF3_DETWIN_SEL  0              0x0
 * 0x005A[25:0]  OOF0_RATIO_REF        0              0x0000000
 * 0x005E[25:0]  OOF1_RATIO_REF        0              0x0000000
 * 0x0062[25:0]  OOF2_RATIO_REF        0              0x0000000
 * 0x0066[25:0]  OOF3_RATIO_REF        0              0x0000000
 * 0x0092[1]     LOL_FST_EN            0              0x0
 * 0x0093[7:4]   LOL_FST_DETWIN_SEL    0              0x0
 * 0x0095[3:2]   LOL_FST_VALWIN_SEL    0              0x0
 * 0x0096[7:4]   LOL_FST_SET_THR_SEL   0              0x0
 * 0x0098[7:4]   LOL_FST_CLR_THR_SEL   0              0x0
 * 0x009A[1]     LOL_SLOW_EN_PLL       0              0x0
 * 0x009B[7:4]   LOL_SLW_DETWIN_SEL    0              0x0
 * 0x009D[3:2]   LOL_SLW_VALWIN_SEL    0              0x0
 * 0x009E[7:4]   LOL_SLW_SET_THR       0              0x0
 * 0x00A0[7:4]   LOL_SLW_CLR_THR       0              0x0
 * 0x00A2[1]     LOL_TIMER_EN          0              0x0
 * 0x00A8[34:0]  LOL_CLR_DELAY         0              0x000000000
 * 0x0102[0]     OUTALL_DISABLE_LOW    1              0x1
 * 0x0108[0]     OUT0_PDN              1              0x1
 * 0x0108[1]     OUT0_OE               0              0x0
 * 0x0108[2]     OUT0_RDIV_FORCE2      0              0x0
 * 0x0109[2:0]   OUT0_FORMAT           1              0x1
 * 0x0109[3]     OUT0_SYNC_EN          1              0x1
 * 0x0109[5:4]   OUT0_DIS_STATE        0              0x0
 * 0x0109[7:6]   OUT0_CMOS_DRV         0              0x0
 * 0x010A[3:0]   OUT0_CM               11             0xB
 * 0x010A[6:4]   OUT0_AMPL             3              0x3
 * 0x010B[2:0]   OUT0_MUX_SEL          0              0x0
 * 0x010B[7:6]   OUT0_INV              0              0x0
 * 0x010D[0]     OUT1_PDN              1              0x1
 * 0x010D[1]     OUT1_OE               0              0x0
 * 0x010D[2]     OUT1_RDIV_FORCE2      0              0x0
 * 0x010E[2:0]   OUT1_FORMAT           1              0x1
 * 0x010E[3]     OUT1_SYNC_EN          1              0x1
 * 0x010E[5:4]   OUT1_DIS_STATE        0              0x0
 * 0x010E[7:6]   OUT1_CMOS_DRV         0              0x0
 * 0x010F[3:0]   OUT1_CM               11             0xB
 * 0x010F[6:4]   OUT1_AMPL             3              0x3
 * 0x0110[2:0]   OUT1_MUX_SEL          0              0x0
 * 0x0110[7:6]   OUT1_INV              0              0x0
 * 0x0112[0]     OUT2_PDN              0              0x0
 * 0x0112[1]     OUT2_OE               1              0x1
 * 0x0112[2]     OUT2_RDIV_FORCE2      1              0x1
 * 0x0113[2:0]   OUT2_FORMAT           1              0x1
 * 0x0113[3]     OUT2_SYNC_EN          1              0x1
 * 0x0113[5:4]   OUT2_DIS_STATE        0              0x0
 * 0x0113[7:6]   OUT2_CMOS_DRV         0              0x0
 * 0x0114[3:0]   OUT2_CM               11             0xB
 * 0x0114[6:4]   OUT2_AMPL             3              0x3
 * 0x0115[2:0]   OUT2_MUX_SEL          0              0x0
 * 0x0115[7:6]   OUT2_INV              0              0x0
 * 0x0117[0]     OUT3_PDN              1              0x1
 * 0x0117[1]     OUT3_OE               0              0x0
 * 0x0117[2]     OUT3_RDIV_FORCE2      0              0x0
 * 0x0118[2:0]   OUT3_FORMAT           1              0x1
 * 0x0118[3]     OUT3_SYNC_EN          1              0x1
 * 0x0118[5:4]   OUT3_DIS_STATE        0              0x0
 * 0x0118[7:6]   OUT3_CMOS_DRV         0              0x0
 * 0x0119[3:0]   OUT3_CM               11             0xB
 * 0x0119[6:4]   OUT3_AMPL             3              0x3
 * 0x011A[2:0]   OUT3_MUX_SEL          0              0x0
 * 0x011A[7:6]   OUT3_INV              0              0x0
 * 0x011C[0]     OUT4_PDN              0              0x0
 * 0x011C[1]     OUT4_OE               1              0x1
 * 0x011C[2]     OUT4_RDIV_FORCE2      1              0x1
 * 0x011D[2:0]   OUT4_FORMAT           1              0x1
 * 0x011D[3]     OUT4_SYNC_EN          1              0x1
 * 0x011D[5:4]   OUT4_DIS_STATE        0              0x0
 * 0x011D[7:6]   OUT4_CMOS_DRV         0              0x0
 * 0x011E[3:0]   OUT4_CM               11             0xB
 * 0x011E[6:4]   OUT4_AMPL             3              0x3
 * 0x011F[2:0]   OUT4_MUX_SEL          1              0x1
 * 0x011F[7:6]   OUT4_INV              0              0x0
 * 0x0121[0]     OUT5_PDN              1              0x1
 * 0x0121[1]     OUT5_OE               0              0x0
 * 0x0121[2]     OUT5_RDIV_FORCE2      0              0x0
 * 0x0122[2:0]   OUT5_FORMAT           1              0x1
 * 0x0122[3]     OUT5_SYNC_EN          1              0x1
 * 0x0122[5:4]   OUT5_DIS_STATE        0              0x0
 * 0x0122[7:6]   OUT5_CMOS_DRV         0              0x0
 * 0x0123[3:0]   OUT5_CM               11             0xB
 * 0x0123[6:4]   OUT5_AMPL             3              0x3
 * 0x0124[2:0]   OUT5_MUX_SEL          0              0x0
 * 0x0124[7:6]   OUT5_INV              0              0x0
 * 0x0126[0]     OUT6_PDN              0              0x0
 * 0x0126[1]     OUT6_OE               1              0x1
 * 0x0126[2]     OUT6_RDIV_FORCE2      1              0x1
 * 0x0127[2:0]   OUT6_FORMAT           1              0x1
 * 0x0127[3]     OUT6_SYNC_EN          1              0x1
 * 0x0127[5:4]   OUT6_DIS_STATE        0              0x0
 * 0x0127[7:6]   OUT6_CMOS_DRV         0              0x0
 * 0x0128[3:0]   OUT6_CM               11             0xB
 * 0x0128[6:4]   OUT6_AMPL             3              0x3
 * 0x0129[2:0]   OUT6_MUX_SEL          1              0x1
 * 0x0129[7:6]   OUT6_INV              0              0x0
 * 0x012B[0]     OUT7_PDN              1              0x1
 * 0x012B[1]     OUT7_OE               0              0x0
 * 0x012B[2]     OUT7_RDIV_FORCE2      0              0x0
 * 0x012C[2:0]   OUT7_FORMAT           1              0x1
 * 0x012C[3]     OUT7_SYNC_EN          1              0x1
 * 0x012C[5:4]   OUT7_DIS_STATE        0              0x0
 * 0x012C[7:6]   OUT7_CMOS_DRV         0              0x0
 * 0x012D[3:0]   OUT7_CM               11             0xB
 * 0x012D[6:4]   OUT7_AMPL             3              0x3
 * 0x012E[2:0]   OUT7_MUX_SEL          0              0x0
 * 0x012E[7:6]   OUT7_INV              0              0x0
 * 0x0130[0]     OUT8_PDN              0              0x0
 * 0x0130[1]     OUT8_OE               1              0x1
 * 0x0130[2]     OUT8_RDIV_FORCE2      1              0x1
 * 0x0131[2:0]   OUT8_FORMAT           1              0x1
 * 0x0131[3]     OUT8_SYNC_EN          1              0x1
 * 0x0131[5:4]   OUT8_DIS_STATE        0              0x0
 * 0x0131[7:6]   OUT8_CMOS_DRV         0              0x0
 * 0x0132[3:0]   OUT8_CM               11             0xB
 * 0x0132[6:4]   OUT8_AMPL             3              0x3
 * 0x0133[2:0]   OUT8_MUX_SEL          0              0x0
 * 0x0133[7:6]   OUT8_INV              0              0x0
 * 0x013A[0]     OUT9_PDN              1              0x1
 * 0x013A[1]     OUT9_OE               0              0x0
 * 0x013A[2]     OUT9_RDIV_FORCE2      0              0x0
 * 0x013B[2:0]   OUT9_FORMAT           1              0x1
 * 0x013B[3]     OUT9_SYNC_EN          1              0x1
 * 0x013B[5:4]   OUT9_DIS_STATE        0              0x0
 * 0x013B[7:6]   OUT9_CMOS_DRV         0              0x0
 * 0x013C[3:0]   OUT9_CM               11             0xB
 * 0x013C[6:4]   OUT9_AMPL             3              0x3
 * 0x013D[2:0]   OUT9_MUX_SEL          0              0x0
 * 0x013D[7:6]   OUT9_INV              0              0x0
 * 0x013F[11:0]  OUTX_ALWAYS_ON        0              0x000
 * 0x0141[1]     OUT_DIS_MSK           0              0x0
 * 0x0141[5]     OUT_DIS_LOL_MSK       0              0x0
 * 0x0141[6]     OUT_DIS_LOSXAXB_MSK   1              0x1
 * 0x0141[7]     OUT_DIS_MSK_LOS_PFD   0              0x0
 * 0x0142[1]     OUT_DIS_MSK_LOL       1              0x1
 * 0x0142[5]     OUT_DIS_MSK_HOLD      1              0x1
 * 0x0202[31:0]  XAXB_FREQ_OFFSET      0              0x00000000
 * 0x0206[1:0]   PXAXB                 0              0x0
 * 0x0208[47:0]  P0_NUM                0              0x000000000000
 * 0x020E[31:0]  P0_DEN                0              0x00000000
 * 0x0212[47:0]  P1_NUM                0              0x000000000000
 * 0x0218[31:0]  P1_DEN                0              0x00000000
 * 0x021C[47:0]  P2_NUM                0              0x000000000000
 * 0x0222[31:0]  P2_DEN                0              0x00000000
 * 0x0226[47:0]  P3_NUM                0              0x000000000000
 * 0x022C[31:0]  P3_DEN                0              0x00000000
 * 0x0231[3:0]   P0_FRACN_MODE         1              0x1
 * 0x0231[4]     P0_FRACN_EN           0              0x0
 * 0x0232[3:0]   P1_FRACN_MODE         1              0x1
 * 0x0232[4]     P1_FRACN_EN           0              0x0
 * 0x0233[3:0]   P2_FRACN_MODE         1              0x1
 * 0x0233[4]     P2_FRACN_EN           0              0x0
 * 0x0234[3:0]   P3_FRACN_MODE         1              0x1
 * 0x0234[4]     P3_FRACN_EN           0              0x0
 * 0x0235[43:0]  MXAXB_NUM             912680550400   0x0D480000000
 * 0x023B[31:0]  MXAXB_DEN             3221225472     0xC0000000
 * 0x024A[23:0]  R0_REG                0              0x000000
 * 0x024D[23:0]  R1_REG                0              0x000000
 * 0x0250[23:0]  R2_REG                0              0x000000
 * 0x0253[23:0]  R3_REG                0              0x000000
 * 0x0256[23:0]  R4_REG                0              0x000000
 * 0x0259[23:0]  R5_REG                0              0x000000
 * 0x025C[23:0]  R6_REG                0              0x000000
 * 0x025F[23:0]  R7_REG                0              0x000000
 * 0x0262[23:0]  R8_REG                0              0x000000
 * 0x0268[23:0]  R9_REG                0              0x000000
 * 0x026B[7:0]   DESIGN_ID0            67             0x43
 * 0x026C[7:0]   DESIGN_ID1            76             0x4C
 * 0x026D[7:0]   DESIGN_ID2            73             0x49
 * 0x026E[7:0]   DESIGN_ID3            67             0x43
 * 0x026F[7:0]   DESIGN_ID4            112            0x70
 * 0x0270[7:0]   DESIGN_ID5            105            0x69
 * 0x0271[7:0]   DESIGN_ID6            120            0x78
 * 0x0272[7:0]   DESIGN_ID7            50             0x32
 * 0x0302[43:0]  N0_NUM                146028888064   0x02200000000
 * 0x0308[31:0]  N0_DEN                2147483648     0x80000000
 * 0x030C[0]     N0_UPDATE             0              0x0
 * 0x030D[43:0]  N1_NUM                45634027520    0x00AA0000000
 * 0x0313[31:0]  N1_DEN                2147483648     0x80000000
 * 0x0317[0]     N1_UPDATE             0              0x0
 * 0x0318[43:0]  N2_NUM                0              0x00000000000
 * 0x031E[31:0]  N2_DEN                0              0x00000000
 * 0x0322[0]     N2_UPDATE             0              0x0
 * 0x0323[43:0]  N3_NUM                0              0x00000000000
 * 0x0329[31:0]  N3_DEN                0              0x00000000
 * 0x032D[0]     N3_UPDATE             0              0x0
 * 0x032E[43:0]  N4_NUM                0              0x00000000000
 * 0x0334[31:0]  N4_DEN                0              0x00000000
 * 0x0338[0]     N4_UPDATE             0              0x0
 * 0x0338[1]     N_UPDATE              0              0x0
 * 0x0339[4:0]   N_FSTEP_MSK           31             0x1F
 * 0x033B[43:0]  N0_FSTEPW             0              0x00000000000
 * 0x0341[43:0]  N1_FSTEPW             0              0x00000000000
 * 0x0347[43:0]  N2_FSTEPW             0              0x00000000000
 * 0x034D[43:0]  N3_FSTEPW             0              0x00000000000
 * 0x0353[43:0]  N4_FSTEPW             0              0x00000000000
 * 0x0359[15:0]  N0_DELAY              0              0x0000
 * 0x035B[15:0]  N1_DELAY              0              0x0000
 * 0x035D[15:0]  N2_DELAY              0              0x0000
 * 0x035F[15:0]  N3_DELAY              0              0x0000
 * 0x0361[15:0]  N4_DELAY              0              0x0000
 * 0x0487[0]     ZDM_EN                0              0x0
 * 0x0487[2:1]   ZDM_IN_SEL            0              0x0
 * 0x0502[4]     ADD_DIV256            0              0x0
 * 0x0508[5:0]   BW0_PLL               0              0x00
 * 0x0509[5:0]   BW1_PLL               0              0x00
 * 0x050A[5:0]   BW2_PLL               0              0x00
 * 0x050B[5:0]   BW3_PLL               0              0x00
 * 0x050C[5:0]   BW4_PLL               0              0x00
 * 0x050D[5:0]   BW5_PLL               0              0x00
 * 0x050E[5:0]   FAST_BW0_PLL          0              0x00
 * 0x050F[5:0]   FAST_BW1_PLL          0              0x00
 * 0x0510[5:0]   FAST_BW2_PLL          0              0x00
 * 0x0511[5:0]   FAST_BW3_PLL          0              0x00
 * 0x0512[5:0]   FAST_BW4_PLL          0              0x00
 * 0x0513[5:0]   FAST_BW5_PLL          0              0x00
 * 0x0515[55:0]  M_NUM                 0              0x00000000000000
 * 0x051C[31:0]  M_DEN                 0              0x00000000
 * 0x0521[3:0]   M_FRAC_MODE           1              0x1
 * 0x0521[4]     M_FRAC_EN             0              0x0
 * 0x0521[5]     PLL_OUT_RATE_SEL      1              0x1
 * 0x052A[0]     IN_SEL_REGCTRL        1              0x1
 * 0x052A[3:1]   IN_SEL                2              0x2
 * 0x052B[0]     FASTLOCK_AUTO_EN      0              0x0
 * 0x052B[1]     FASTLOCK_MAN          0              0x0
 * 0x052C[0]     HOLD_EN               1              0x1
 * 0x052C[3]     HOLD_RAMP_BYP         1              0x1
 * 0x052C[7:5]   HOLD_RAMP_RATE        0              0x0
 * 0x052D[1]     HOLD_RAMPBYP_NOHIST   1              0x1
 * 0x052E[4:0]   HOLD_HIST_LEN         0              0x00
 * 0x052F[4:0]   HOLD_HIST_DELAY       0              0x00
 * 0x0531[4:0]   HOLD_REF_COUNT_FRC    0              0x00
 * 0x0532[23:0]  HOLD_15M_CYC_COUNT    1024           0x000400
 * 0x0535[0]     FORCE_HOLD            1              0x1
 * 0x0536[1:0]   CLK_SWITCH_MODE       0              0x0
 * 0x0536[2]     HSW_EN                1              0x1
 * 0x0536[3]     HSW_RAMP_BYP          1              0x1
 * 0x0537[3:0]   IN_LOS_MSK            0              0x0
 * 0x0537[7:4]   IN_OOF_MSK            0              0x0
 * 0x0538[2:0]   IN0_PRIORITY          0              0x0
 * 0x0538[6:4]   IN1_PRIORITY          0              0x0
 * 0x0539[2:0]   IN2_PRIORITY          0              0x0
 * 0x0539[6:4]   IN3_PRIORITY          0              0x0
 * 0x0802[15:0]  FIXREGSA0             1333           0x0535
 * 0x0804[7:0]   FIXREGSD0             1              0x01
 * 0x090E[0]     XAXB_EXTCLK_EN        0              0x0
 * 0x0943[0]     IO_VDD_SEL            1              0x1
 * 0x0949[3:0]   IN_EN                 0              0x0
 * 0x0949[7:4]   IN_PULSED_CMOS_EN     0              0x0
 * 0x094A[3:0]   INX_TO_PFD_EN         0              0x0
 * 0x0A02[4:0]   N_ADD_0P5             0              0x00
 * 0x0A03[4:0]   N_CLK_TO_OUTX_EN      3              0x03
 * 0x0A04[4:0]   N_PIBYP               1              0x01
 * 0x0A05[4:0]   N_PDNB                3              0x03
 * 0x0B44[3:0]   PDIV_FRACN_CLK_DIS    15             0xF
 * 0x0B44[5]     FRACN_CLK_DIS_PLL     0              0x0
 * 0x0B46[3:0]   LOS_CLK_DIS           0              0x0
 * 0x0B47[4:0]   OOF_CLK_DIS           0              0x00
 * 0x0B48[4:0]   OOF_DIV_CLK_DIS       15             0x0F
 * 0x0B4A[4:0]   N_CLK_DIS             28             0x1C
 *
 *
 */

#endif
