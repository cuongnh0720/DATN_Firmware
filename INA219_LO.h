#ifndef _INA219_LO_H_
#define _INA219_LO_H_

// Include statements
#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

// I2C Interface
#define INA219_LO_SCL(x)    IOWR_ALTERA_AVALON_PIO_DATA(INA219_LO_SCL_BASE, (x ? 1 : 0))
#define INA219_LO_SDA_IN()  IOWR_ALTERA_AVALON_PIO_DIRECTION(INA219_LO_SDA_BASE, 0)
#define INA219_LO_SDA_GET() IORD_ALTERA_AVALON_PIO_DATA(INA219_LO_SDA_BASE)
#define INA219_LO_SDA_OUT() IOWR_ALTERA_AVALON_PIO_DIRECTION(INA219_LO_SDA_BASE, 1)
#define INA219_LO_SDA(x)    IOWR_ALTERA_AVALON_PIO_DATA(INA219_LO_SDA_BASE, (x ? 1 : 0))

/** Default I2C address **/
#define INA219_LO_ADDRESS (0x40) // 1000000 (A0+A1=GND)

/** Read operation **/
#define INA219_LO_READ (0x01)

/** Config register address **/
#define INA219_LO_REG_CONFIG (0x00)

/** Reset bit **/
#define INA219_LO_CONFIG_RESET (0x8000) // Reset Bit

/** Mask for bus voltage range **/
#define INA219_LO_CONFIG_BVOLTAGERANGE_MASK (0x2000) // Bus Voltage Range Mask

/** Bus voltage range values **/
typedef enum
{
    INA219_LO_CONFIG_BVOLTAGERANGE_16V = (0x0000), // 0-16V Range
    INA219_LO_CONFIG_BVOLTAGERANGE_32V = (0x2000), // 0-32V Range
} INA219_LO_BusVoltageRange;

/** Mask for gain bits **/
#define INA219_LO_CONFIG_GAIN_MASK (0x1800) // Gain Mask

/** Values for gain bits **/
typedef enum
{
    INA219_LO_CONFIG_GAIN_1_40MV = (0x0000),  // Gain 1, 40mV Range
    INA219_LO_CONFIG_GAIN_2_80MV = (0x0800),  // Gain 2, 80mV Range
    INA219_LO_CONFIG_GAIN_4_160MV = (0x1000), // Gain 4, 160mV Range
    INA219_LO_CONFIG_GAIN_8_320MV = (0x1800), // Gain 8, 320mV Range
} INA219_LO_Gain;

/** Mask for bus ADC resolution bits **/
#define INA219_LO_CONFIG_BADCRES_MASK (0x0780)

/** Values for bus ADC resolution **/
typedef enum
{
    INA219_LO_CONFIG_BADCRES_9BIT = (0x0000),             // 9-bit bus res = 0..511
    INA219_LO_CONFIG_BADCRES_10BIT = (0x0080),            // 10-bit bus res = 0..1023
    INA219_LO_CONFIG_BADCRES_11BIT = (0x0100),            // 11-bit bus res = 0..2047
    INA219_LO_CONFIG_BADCRES_12BIT = (0x0180),            // 12-bit bus res = 0..4097
    INA219_LO_CONFIG_BADCRES_12BIT_2S_1060US = (0x0480),  // 2 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_4S_2130US = (0x0500),  // 4 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_8S_4260US = (0x0580),  // 8 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_16S_8510US = (0x0600), // 16 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_32S_17MS = (0x0680),   // 32 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_64S_34MS = (0x0700),   // 64 x 12-bit bus samples averaged together
    INA219_LO_CONFIG_BADCRES_12BIT_128S_69MS = (0x0780),  // 128 x 12-bit bus samples averaged together
} INA219_LO_BusADCResolution;

/** Mask for shunt ADC resolution bits **/
#define INA219_LO_CONFIG_SADCRES_MASK (0x0078) // Shunt ADC Resolution and Averaging Mask

/** Values for shunt ADC resolution **/
typedef enum
{
    INA219_LO_CONFIG_SADCRES_9BIT_1S_84US = (0x0000),     // 1 x 9-bit shunt sample
    INA219_LO_CONFIG_SADCRES_10BIT_1S_148US = (0x0008),   // 1 x 10-bit shunt sample
    INA219_LO_CONFIG_SADCRES_11BIT_1S_276US = (0x0010),   // 1 x 11-bit shunt sample
    INA219_LO_CONFIG_SADCRES_12BIT_1S_532US = (0x0018),   // 1 x 12-bit shunt sample
    INA219_LO_CONFIG_SADCRES_12BIT_2S_1060US = (0x0048),  // 2 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_4S_2130US = (0x0050),  // 4 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_8S_4260US = (0x0058),  // 8 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_16S_8510US = (0x0060), // 16 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_32S_17MS = (0x0068),   // 32 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_64S_34MS = (0x0070),   // 64 x 12-bit shunt samples averaged together
    INA219_LO_CONFIG_SADCRES_12BIT_128S_69MS = (0x0078),  // 128 x 12-bit shunt samples averaged together
} INA219_LO_ShuntADCResolution;

/** Mask for operating mode bits **/
#define INA219_LO_CONFIG_MODE_MASK (0x0007) // Operating Mode Mask

/** Values for operating mode **/
typedef enum
{
    INA219_LO_CONFIG_MODE_POWERDOWN = 0x00,           /**< power down */
    INA219_LO_CONFIG_MODE_SVOLT_TRIGGERED = 0x01,     /**< shunt voltage triggered */
    INA219_LO_CONFIG_MODE_BVOLT_TRIGGERED = 0x02,     /**< bus voltage triggered */
    INA219_LO_CONFIG_MODE_SANDBVOLT_TRIGGERED = 0x03, /**< shunt and bus voltage triggered */
    INA219_LO_CONFIG_MODE_ADCOFF = 0x04,              /**< ADC off */
    INA219_LO_CONFIG_MODE_SVOLT_CONTINUOUS = 0x05,    /**< shunt voltage continuous */
    INA219_LO_CONFIG_MODE_BVOLT_CONTINUOUS = 0x06,    /**< bus voltage continuous */
    INA219_LO_CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0x07 /**< shunt and bus voltage continuous */
} INA219_LO_OperatingMode;

/** Register addresses **/
#define INA219_LO_REG_SHUNTVOLTAGE (0x01)
#define INA219_LO_REG_BUSVOLTAGE (0x02)
#define INA219_LO_REG_POWER (0x03)
#define INA219_LO_REG_CURRENT (0x04)
#define INA219_LO_REG_CALIBRATION (0x05)

/** Function prototypes **/
bool INA219_LO_Begin(void);
void INA219_LO_SetCalibration_32V_2A(void);
void INA219_LO_SetCalibration_32V_1A(void);
void INA219_LO_SetCalibration_16V_400mA(void);
float INA219_LO_GetBusVoltage_V(void);
float INA219_LO_GetShuntVoltage_MV(void);
float INA219_LO_GetCurrent_MA(void);
float INA219_LO_GetPower_MW(void);
void INA219_LO_PowerSave(bool on);

#endif
