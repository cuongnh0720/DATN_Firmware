#ifndef _SHT3x_H_
#define _SHT3x_H_

// Include statements
#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

// I2C interface
#define SCL(x)     IOWR_ALTERA_AVALON_PIO_DATA(SHT31_SCL_BASE, (x ? 1 : 0))
#define SDA_IN()   IOWR_ALTERA_AVALON_PIO_DIRECTION(SHT31_SDA_BASE, 0)
#define SDA_GET()  IORD_ALTERA_AVALON_PIO_DATA(SHT31_SDA_BASE)
#define SDA_OUT()  IOWR_ALTERA_AVALON_PIO_DIRECTION(SHT31_SDA_BASE, 1)
#define SDA(x)     IOWR_ALTERA_AVALON_PIO_DATA(SHT31_SDA_BASE, (x ? 1 : 0))

extern double temperature, humidity;

#define u8 unsigned char

// Function prototyes
char SHT3x_Read(uint16_t dat);

#endif
