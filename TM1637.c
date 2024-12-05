#include "TM1637.h"

const uint8_t segmentMap[] = {
    // XGFEDCBA
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
};

void TM1637_I2C_Start(void)
{

    CLK_HIGH();
    DATA_HIGH();
    usleep(2);
    DATA_LOW();
}

void TM1637_I2C_Stop(void)
{
    CLK_LOW();
    usleep(2);
    DATA_LOW();
    usleep(2);
    CLK_HIGH();
    usleep(2);
    DATA_HIGH();
}

void TM1637_I2C_WaitForAck(void)
{
    CLK_LOW();
    usleep(5); // After the falling edge of the eighth clock delay 5us
               // ACK signals the beginning of judgment
               // while (dio); // Check the state of the Data pin
    CLK_HIGH();
    usleep(2);
    CLK_LOW();
}

void TM1637_I2C_WriteByte(uint8_t byte)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        CLK_LOW();
        if (byte & 0x01) // low front
        {
            DATA_HIGH();
        }
        else
        {
            DATA_LOW();
        }
        usleep(3);
        byte = byte >> 1;
        CLK_HIGH();
        usleep(3);
    }
}

void TM1637_WriteData_AddressAutoMode(uint8_t Addr, uint8_t *data, int size)
{
    TM1637_I2C_Start();
    TM1637_I2C_WriteByte(0x40);
    TM1637_I2C_WaitForAck();
    TM1637_I2C_Stop();

    TM1637_I2C_Start();
    TM1637_I2C_WriteByte(Addr);
    TM1637_I2C_WaitForAck();
    for (int i = 0; i < size; i++)
    {
        TM1637_I2C_WriteByte(data[i]);
        TM1637_I2C_WaitForAck();
    }
    TM1637_I2C_Stop();

    TM1637_I2C_Start();
    TM1637_I2C_WriteByte(0x8A);
    TM1637_I2C_WaitForAck();
    TM1637_I2C_Stop();
}

void TM1637_WriteData_FixedAddress(uint8_t Addr, uint8_t *data, int size)
{
    TM1637_I2C_Start();
    TM1637_I2C_WriteByte(0x44);
    TM1637_I2C_WaitForAck();
    TM1637_I2C_Stop();

    for (int i = 0; i < size; i++)
    {
        TM1637_I2C_Start();
        TM1637_I2C_WriteByte(Addr + (size - 1 - i));
        TM1637_I2C_WaitForAck();
        TM1637_I2C_WriteByte(data[i]);
        TM1637_I2C_WaitForAck();
        TM1637_I2C_Stop();
    }

    TM1637_I2C_Start();
    TM1637_I2C_WriteByte(0x8A);
    TM1637_I2C_WaitForAck();
    TM1637_I2C_Stop();
}

void TM1637_WriteNum_AddressAutoMode(uint8_t Addr, int num, int colon)
{
	static int dColon = 0;
	static int sChange = 0;
	int num_reg = num;
//    uint8_t buffer[4] = {0b00111111, 0b00111111, 0b00111111, 0b00111111};
    uint8_t buffer[4];
//    TM1637_WriteData_AddressAutoMode(Addr, buffer, 4);
    int len;

    if ((num / 1000) != 0)
        len = 4;
    else if ((num / 100) != 0)
        len = 3;
    else if ((num / 10) != 0)
        len = 2;
    else
        len = 1;

    if (len < 4)
    {
        for (int i = 0; i < 4; i++)
        {
            buffer[3 - i] = segmentMap[num % 10];
            num = num / 10;
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            buffer[len - 1 - i] = segmentMap[num % 10];
            num = num / 10;
        }
    }

//    if (colon)
//    {
//        buffer[1] |= 1 << 7; // turn on the colon
//    }
//    TM1637_WriteData_AddressAutoMode(Addr, buffer, len);

    if (sChange != num_reg) // if the unit digit in the time changes
    {
       if (colon)
          dColon = !dColon;
       if (dColon)
          buffer[1] |= 1 << 7;
       sChange = num_reg; // update the sChange with current value of the unit digit
       TM1637_WriteData_AddressAutoMode(Addr, buffer, 4);
    }

}

void TM1637_WriteNum_FixedAddress(uint8_t Addr, int num)
{
    uint8_t buffer[4];

    for (int i = 0; i < 4; i++)
    {
        buffer[i] = segmentMap[num % 10];
        num = num / 10;
    }

    TM1637_WriteData_FixedAddress(Addr, buffer, 4);
}

void TM1637_WriteTime(uint8_t *time, int colon)
{
    static int dColon = 0;
    static int sChange = 0;
    uint8_t buffer[4];

    for (int i = 0; i < 4; i++)
    {
        buffer[3 - i] = segmentMap[time[i]];
    }

    if (sChange != time[0]) // if the unit digit in the time changes
    {
        if (colon)
            dColon = !dColon;
        if (dColon)
            buffer[1] |= 1 << 7;
        sChange = time[0]; // update the sChange with current value of the unit digit
        TM1637_WriteData_AddressAutoMode(0xC0, buffer, 4);
    }
}
