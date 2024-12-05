#ifndef _TM1637_H_
#define _TM1637_H_

// Include statements
#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

// I2C inteface
#define CLK_HIGH() 	IOWR_ALTERA_AVALON_PIO_DATA(TM1637_CLK_BASE, 1)
#define CLK_LOW() 	IOWR_ALTERA_AVALON_PIO_DATA(TM1637_CLK_BASE, 0)
#define DATA_HIGH() IOWR_ALTERA_AVALON_PIO_DATA(TM1637_SDA_BASE, 1)
#define DATA_LOW() 	IOWR_ALTERA_AVALON_PIO_DATA(TM1637_SDA_BASE, 0)

// Function prototypes
void TM1637_WriteData_AddressAutoMode(uint8_t addr, uint8_t *data, int size);
void TM1637_writeData_FixedAddress(uint8_t addr, uint8_t *data, int size);
void TM1637_WriteNum_AddressAutoMode(uint8_t addr, int num, int colon);
void TM1637_WriteNum_FixedAddress(uint8_t addr, int num);
void TM1637_WriteTime(uint8_t *time, int colon);

#endif
