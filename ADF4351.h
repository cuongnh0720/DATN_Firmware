#ifndef _ADF4351_H_
#define _ADF4351_H_

// Include statements
#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <unistd.h>
#include <stdint.h>

// SPI interface
#define ADF4351_CLK(x)         IOWR_ALTERA_AVALON_PIO_DATA(ADF4351_CLK_BASE, (x ? 1 : 0))
#define DAT_IN()               IOWR_ALTERA_AVALON_PIO_DIRECTION(ADF4351_DAT_BASE, 0)
#define DAT_OUT()              IOWR_ALTERA_AVALON_PIO_DIRECTION(ADF4351_DAT_BASE, 1)
#define ADF4351_INPUT_DATA()   IORD_ALTERA_AVALON_PIO_DATA(ADF4351_DAT_BASE)
#define ADF4351_OUTPUT_DATA(x) IOWR_ALTERA_AVALON_PIO_DATA(ADF4351_DAT_BASE, (x ? 1 : 0))
#define ADF4351_CE(x)          IOWR_ALTERA_AVALON_PIO_DATA(ADF4351_CE_BASE, (x ? 1 : 0))
#define ADF4351_LE(x)          IOWR_ALTERA_AVALON_PIO_DATA(ADF4351_LE_BASE, (x ? 1 : 0))

// Register config
#define ADF4351_R0 			   ((uint32_t)0X002C8018)
#define ADF4351_R1 			   ((uint32_t)0X00008029)
#define ADF4351_R2 			   ((uint32_t)0X00010E42)
#define ADF4351_R3 			   ((uint32_t)0X000004B3)
#define ADF4351_R4 			   ((uint32_t)0X00EC803C)
#define ADF4351_R5 			   ((uint32_t)0X00580005)

#define ADF4351_R1_BASE 	   ((uint32_t)0X00008001)
#define ADF4351_R4_BASE 	   ((uint32_t)0X008C803C)
#define ADF4351_R4_ON 		   ((uint32_t)0X008C803C)
#define ADF4351_R4_OFF 		   ((uint32_t)0X008C883C)

#define ADF4351_RF_OFF         ((uint32_t)0X00EC801C)

#define ADF4351_PD_ON          ((uint32_t)0X00010E42)
#define ADF4351_PD_OFF         ((uint32_t)0X00010E02)

#define OUTPUT_POWER_MAX_LV1   ((int8_t)0x00)
#define OUTPUT_POWER_MAX_LV2   ((int8_t)0x01)
#define OUTPUT_POWER_MAX_LV3   ((int8_t)0x10)
#define OUTPUT_POWER_MAX_LV4   ((int8_t)0x11)

// Function prototypes
void ADF4351Init(void);
void writeToADF4351(uint8_t count, uint8_t *buf);
void writeOneRegToADF4351(uint32_t reg);
void ADF4351WriteFreq(float freq);
uint32_t ADF4351WriteMaxOutputPower(uint32_t config_r4, int8_t lev);

#endif
