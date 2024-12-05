#include "SHT3x.h"

double temperature = 0.0, humidity = 0.0;

void SHT3x_I2C_Start(void)
{
    SDA_OUT();
    SCL(1);
    SDA(0);

    SDA(1);
    usleep(5);
    SDA(0);
    usleep(5);

    SCL(0);
}

void SHT3x_I2C_Stop(void)
{
    SDA_OUT();
    SCL(0);
    SDA(0);

    SCL(1);
    usleep(5);
    SDA(1);
    usleep(5);
}

void SHT3x_I2C_Send_Ack(unsigned char ack)
{
    SDA_OUT();
    SCL(0);
    SDA(0);
    usleep(5);
    if (!ack)
        SDA(0);
    else
        SDA(1);
    SCL(1);
    usleep(5);
    SCL(0);
    SDA(1);
}

unsigned char SHT3x_I2C_waitAck(void)
{
    char ack = 0;
    unsigned char ack_flag = 10;
    SCL(0);
    SDA(1);
    SDA_IN();

    SCL(1);
    while ((SDA_GET() == 1) && (ack_flag))
    {
        ack_flag--;
        usleep(5);
    }

    if (ack_flag <= 0)
    {
        SHT3x_I2C_Stop();
        return 1;
    }
    else
    {
        SCL(0);
        SDA_OUT();
    }
    return ack;
}

void SHT3x_Send_Byte(u8 dat)
{
    int i = 0;
    SDA_OUT();
    SCL(0);

    for (i = 0; i < 8; i++)
    {
        SDA((dat & 0x80) >> 7);
        __asm__("nop");
        SCL(1);
        usleep(5);
        SCL(0);
        usleep(5);
        dat <<= 1;
    }
}

unsigned char SHT3x_Read_Byte(void)
{
    unsigned char i, receive = 0;
    SDA_IN();
    for (i = 0; i < 8; i++)
    {
        SCL(0);
        usleep(5);
        SCL(1);
        usleep(5);
        receive <<= 1;
        if (SDA_GET())
        {
            receive |= 1;
        }
        usleep(5);
    }
    SCL(0);
    return receive;
}

char SHT3x_Write_Mode(uint16_t dat)
{
    SHT3x_I2C_Start();

    SHT3x_Send_Byte((0X44 << 1) | 0);
    if (SHT3x_I2C_waitAck() == 1)
        return 1;
    SHT3x_Send_Byte((dat >> 8));
    if (SHT3x_I2C_waitAck() == 1)
        return 2;
    SHT3x_Send_Byte(dat & 0xff);
    if (SHT3x_I2C_waitAck() == 1)
        return 3;
    return 0;
}

unsigned char SHT3x_CRC8(const unsigned char *data, int len)
{
    const unsigned char POLYNOMIAL = 0x31;
    unsigned char crc = 0xFF;
    int j, i;

    for (j = 0; j < len; j++)
    {
        crc ^= *data++;
        for (i = 0; i < 8; i++)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}

char SHT3x_Read(uint16_t dat)
{
    uint16_t i = 0;
    unsigned char buff[6] = {0};
    uint16_t data_16 = 0;

    SHT3x_Write_Mode(0x2130);

    SHT3x_I2C_Start();
    SHT3x_Send_Byte((0x44 << 1) | 0);
    if (SHT3x_I2C_waitAck() == 1)
        return 1;
    SHT3x_Send_Byte((dat >> 8));
    if (SHT3x_I2C_waitAck() == 1)
        return 2;
    SHT3x_Send_Byte(dat & 0xFF);
    if (SHT3x_I2C_waitAck() == 1)
        return 3;

    do
    {
        i++;
        if (i > 20)
            return 4;
        usleep(2000);

        SHT3x_I2C_Start();
        SHT3x_Send_Byte((0X44 << 1) | 1);
    } while (SHT3x_I2C_waitAck() == 1);

    buff[0] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(0);

    buff[1] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(0);

    buff[2] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(0);

    buff[3] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(0);

    buff[4] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(0);

    buff[5] = SHT3x_Read_Byte();
    SHT3x_I2C_Send_Ack(1);

    SHT3x_I2C_Stop();

    if ((SHT3x_CRC8(buff, 2) == buff[2]) && (SHT3x_CRC8(buff + 3, 2) == buff[5]))
    {
        data_16 = (buff[0] << 8) | buff[1];
        temperature = (data_16 / 65535.0) * 175.0 - 45;
        data_16 = 0;
        data_16 = (buff[3] << 8) | buff[4];
        humidity = (data_16 / 65535.0) * 100.0;

//        printf("temp = %.2f\r\n", Temperature);
//        printf("humi = %.2f\r\n", Humidity);
        return 0;
    }
    // else
    // {
    //    printf("Verification failed\r\n");
    // }
    return 5;
}
