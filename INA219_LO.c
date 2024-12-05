#include "INA219_LO.h"

static uint16_t ina219_lo_currentDivider_mA;
static float ina219_lo_powerMultiplier_mW;
static uint8_t ina219_lo_i2caddr = -1;
static uint32_t ina219_lo_calValue;

// Ham bat dau giao tiep I2C
static void INA219_LO_I2C_Start()
{
    INA219_LO_SDA_OUT();
    INA219_LO_SDA(1);
    INA219_LO_SCL(1);
    usleep(1);
    INA219_LO_SDA(0);
    usleep(1);
    INA219_LO_SCL(0);
}

// Ham dung giao tiep I2C
static void INA219_LO_I2C_Stop()
{
    INA219_LO_SDA_OUT();
    INA219_LO_SCL(0);
    INA219_LO_SDA(0);
    usleep(1);
    INA219_LO_SCL(1);
    usleep(1);
    INA219_LO_SDA(1);
    usleep(1);
}

// Ham ghi 1 bit
static void INA219_LO_I2C_WriteBit(uint8_t bit)
{
    INA219_LO_SDA_OUT();
    if (bit)
    {
        INA219_LO_SDA(1);
    }
    else
    {
        INA219_LO_SDA(0);
    }
    usleep(1);
    INA219_LO_SCL(1);
    usleep(1);
    INA219_LO_SCL(0);
    usleep(1);
}

// Ham ghi 1 byte
static uint8_t INA219_LO_I2C_WriteByte(uint8_t byte)
{
    INA219_LO_SDA_OUT();
    for (int i = 0; i < 8; i++)
    {
        INA219_LO_I2C_WriteBit((byte << i) & 0x80);
    }
    // Doc ACK
    INA219_LO_SDA(1);
    usleep(1);
    INA219_LO_SCL(1);
    INA219_LO_SDA_IN();
    uint8_t ack = INA219_LO_SDA_GET();
    INA219_LO_SCL(0);
    return ack;
}

// Ham doc 1 byte
static uint8_t INA219_LO_I2C_ReadByte(uint8_t ack)
{
    INA219_LO_SDA_OUT();
    uint8_t byte = 0;
    INA219_LO_SDA(1);
    INA219_LO_SDA_IN();
    for (int i = 0; i < 8; i++)
    {
        INA219_LO_SCL(1);
        usleep(1);
        byte = (byte << 1) | INA219_LO_SDA_GET();
        INA219_LO_SCL(0);
        usleep(1);
    }
    INA219_LO_I2C_WriteBit(ack ? 0 : 1); // Gui ACK hoac NACK
    return byte;
}

// Ham khoi tao INA219
static void INA219_LO()
{
    ina219_lo_i2caddr = INA219_LO_ADDRESS;
    ina219_lo_currentDivider_mA = 0;
    ina219_lo_powerMultiplier_mW = 0.0f;
}

// Ham khoi tao INA219
static void INA219_LO_Init()
{
    // Set chip voi cau hinh mac dinh
    INA219_LO_SetCalibration_32V_2A();
}

// Ham bat dau giao tiep voi INA219
bool INA219_LO_Begin()
{
    INA219_LO();
    INA219_LO_I2C_Start();
    if (INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0) != 0)
    { // Gui dia chi + bit ghi
        INA219_LO_I2C_Stop();
        return false;
    }
    INA219_LO_I2C_Stop();
    INA219_LO_Init();
    return true;
}

// Ham lay gia tri bus voltage raw
static int16_t INA219_LO_GetBusVoltage_Raw()
{
    uint16_t value = 0;
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_BUSVOLTAGE);     // Gui register can doc
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 1); // Gui dia chi + bit doc
    value = (INA219_LO_I2C_ReadByte(1) << 8) | INA219_LO_I2C_ReadByte(0);
    INA219_LO_I2C_Stop();

    // Dich phai 3 bit va nhan voi 4 de bo qua CNVR va OVF
    return (int16_t)((value >> 3) * 4);
}

// Ham lay gia tri shunt voltage raw
static int16_t INA219_LO_GetShuntVoltage_Raw()
{
    uint16_t value = 0;
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_SHUNTVOLTAGE);   // Gui register can doc
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 1); // Gui dia chi + bit doc
    value = (INA219_LO_I2C_ReadByte(1) << 8) | INA219_LO_I2C_ReadByte(0);
    INA219_LO_I2C_Stop();

    return value;
}

// Ham lay gia tri current raw
static int16_t INA219_LO_GetCurrent_Raw()
{
    uint16_t value = 0;
    // Thiet lap gia tri calibration de dam bao CURRENT va POWER co the doc duoc
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0);     // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CALIBRATION);        // Gui register calibration
    INA219_LO_I2C_WriteByte((ina219_lo_calValue >> 8) & 0xFF); // Gui byte cao
    INA219_LO_I2C_WriteByte(ina219_lo_calValue & 0xFF);        // Gui byte thap
    INA219_LO_I2C_Stop();

    // Doc gia tri current
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CURRENT);        // Gui register current
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 1); // Gui dia chi + bit doc
    value = (INA219_LO_I2C_ReadByte(1) << 8) | INA219_LO_I2C_ReadByte(0);
    INA219_LO_I2C_Stop();

    return value;
}

// Ham lay gia tri power raw
static int16_t INA219_LO_GetPower_Raw()
{
    uint16_t value = 0;
    // Thiet lap gia tri calibration de dam bao CURRENT va POWER co the doc duoc
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0);     // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CALIBRATION);        // Gui register calibration
    INA219_LO_I2C_WriteByte((ina219_lo_calValue >> 8) & 0xFF); // Gui byte cao
    INA219_LO_I2C_WriteByte(ina219_lo_calValue & 0xFF);        // Gui byte thap
    INA219_LO_I2C_Stop();

    // Doc gia tri power
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_POWER);          // Gui register power
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 1); // Gui dia chi + bit doc
    value = (INA219_LO_I2C_ReadByte(1) << 8) | INA219_LO_I2C_ReadByte(0);
    INA219_LO_I2C_Stop();

    return value;
}

// Ham cau hinh de do dien ap den 32V va dong den 2A
void INA219_LO_SetCalibration_32V_2A()
{
    ina219_lo_calValue = 4096;

    // Thiet lap gia tri chia de chuyen doi gia tri dong va cong suat
    ina219_lo_currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
    ina219_lo_powerMultiplier_mW = 2; // Power LSB = 1mW per bit (2/1)

    // Ghi gia tri calibration vao thanh ghi Calibration
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0);     // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CALIBRATION);        // Gui register calibration
    INA219_LO_I2C_WriteByte((ina219_lo_calValue >> 8) & 0xFF); // Gui byte cao
    INA219_LO_I2C_WriteByte(ina219_lo_calValue & 0xFF);        // Gui byte thap
    INA219_LO_I2C_Stop();

    // Cau hinh thanh ghi Config
    uint16_t config = INA219_LO_CONFIG_BVOLTAGERANGE_32V |
                      INA219_LO_CONFIG_GAIN_8_320MV | INA219_LO_CONFIG_BADCRES_12BIT |
                      INA219_LO_CONFIG_SADCRES_12BIT_1S_532US |
                      INA219_LO_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CONFIG);         // Gui register config
    INA219_LO_I2C_WriteByte((config >> 8) & 0xFF);         // Gui byte cao
    INA219_LO_I2C_WriteByte(config & 0xFF);                // Gui byte thap
    INA219_LO_I2C_Stop();
}

// Ham cau hinh de do dien ap den 32V va dong den 1A
void INA219_LO_SetCalibration_32V_1A()
{
    ina219_lo_calValue = 10240;

    // Thiet lap gia tri chia de chuyen doi gia tri dong va cong suat
    ina219_lo_currentDivider_mA = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
    ina219_lo_powerMultiplier_mW = 0.8f; // Power LSB = 800uW per bit

    // Ghi gia tri calibration vao thanh ghi Calibration
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0);     // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CALIBRATION);        // Gui register calibration
    INA219_LO_I2C_WriteByte((ina219_lo_calValue >> 8) & 0xFF); // Gui byte cao
    INA219_LO_I2C_WriteByte(ina219_lo_calValue & 0xFF);        // Gui byte thap
    INA219_LO_I2C_Stop();

    // Cau hinh thanh ghi Config
    uint16_t config = INA219_LO_CONFIG_BVOLTAGERANGE_32V |
                      INA219_LO_CONFIG_GAIN_8_320MV | INA219_LO_CONFIG_BADCRES_12BIT |
                      INA219_LO_CONFIG_SADCRES_12BIT_1S_532US |
                      INA219_LO_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CONFIG);         // Gui register config
    INA219_LO_I2C_WriteByte((config >> 8) & 0xFF);         // Gui byte cao
    INA219_LO_I2C_WriteByte(config & 0xFF);                // Gui byte thap
    INA219_LO_I2C_Stop();
}

// Ham cau hinh de do dien ap den 16V va dong den 400mA
void INA219_LO_SetCalibration_16V_400mA()
{
    ina219_lo_calValue = 8192;

    // Thiet lap gia tri chia de chuyen doi gia tri dong va cong suat
    ina219_lo_currentDivider_mA = 20;    // Current LSB = 50uA per bit (1000/50 = 20)
    ina219_lo_powerMultiplier_mW = 1.0f; // Power LSB = 1mW per bit

    // Ghi gia tri calibration vao thanh ghi Calibration
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0);     // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CALIBRATION);        // Gui register calibration
    INA219_LO_I2C_WriteByte((ina219_lo_calValue >> 8) & 0xFF); // Gui byte cao
    INA219_LO_I2C_WriteByte(ina219_lo_calValue & 0xFF);        // Gui byte thap
    INA219_LO_I2C_Stop();

    // Cau hinh thanh ghi Config
    uint16_t config = INA219_LO_CONFIG_BVOLTAGERANGE_16V |
                      INA219_LO_CONFIG_GAIN_1_40MV | INA219_LO_CONFIG_BADCRES_12BIT |
                      INA219_LO_CONFIG_SADCRES_12BIT_1S_532US |
                      INA219_LO_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CONFIG);         // Gui register config
    INA219_LO_I2C_WriteByte((config >> 8) & 0xFF);         // Gui byte cao
    INA219_LO_I2C_WriteByte(config & 0xFF);                // Gui byte thap
    INA219_LO_I2C_Stop();
}

// Ham lay gia tri bus voltage (V)
float INA219_LO_GetBusVoltage_V()
{
    int16_t value = INA219_LO_GetBusVoltage_Raw();
    return value * 0.001;
}

// Ham lay gia tri shunt voltage (mV)
float INA219_LO_GetShuntVoltage_MV()
{
    int16_t value = INA219_LO_GetShuntVoltage_Raw();
    return value * 0.01;
}

// Ham lay gia tri current (mA)
float INA219_LO_GetCurrent_MA()
{
    float valueDec = INA219_LO_GetCurrent_Raw();
    valueDec /= ina219_lo_currentDivider_mA;
    return valueDec;
}

// Ham lay gia tri power (mW)
float INA219_LO_GetPower_MW()
{
    float valueDec = INA219_LO_GetPower_Raw();
    valueDec *= ina219_lo_powerMultiplier_mW;
    return valueDec;
}

// Ham cau hinh che do tiet kiem nang luong
void INA219_LO_PowerSave(bool on)
{
    uint16_t config;
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CONFIG);         // Gui register config
    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 1); // Gui dia chi + bit doc
    config = (INA219_LO_I2C_ReadByte(1) << 8) | INA219_LO_I2C_ReadByte(0);
    INA219_LO_I2C_Stop();

    if (on)
    {
        config &= ~(0x0007); // Thiet lap che do POWERDOWN
        config |= INA219_LO_CONFIG_MODE_POWERDOWN;
    }
    else
    {
        config &= ~(0x0007); // Thiet lap che do SANDBVOLT_CONTINUOUS
        config |= INA219_LO_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
    }

    INA219_LO_I2C_Start();
    INA219_LO_I2C_WriteByte((ina219_lo_i2caddr << 1) | 0); // Gui dia chi + bit ghi
    INA219_LO_I2C_WriteByte(INA219_LO_REG_CONFIG);         // Gui register config
    INA219_LO_I2C_WriteByte((config >> 8) & 0xFF);         // Gui byte cao
    INA219_LO_I2C_WriteByte(config & 0xFF);                // Gui byte thap
    INA219_LO_I2C_Stop();
}


