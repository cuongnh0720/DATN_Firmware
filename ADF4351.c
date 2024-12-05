#include "ADF4351.h"

extern int8_t level_reg;

void ADF4351Init(void)
{
	// Register 32 bit:
    uint8_t buffer[4] = {0, 0, 0, 0};

    // Init Register R5:
    buffer[3] = 0x00;
    buffer[2] = 0x58;
    buffer[1] = 0x00; // write communication register 0x00580005 to control the progress
    buffer[0] = 0x05; // to write Register 5 to set digital lock detector
    writeToADF4351(4, buffer);

    // Init Register R4:
    buffer[3] = 0x00;
    buffer[2] = 0xEC; //(DB23=1)The signal is taken from the VCO directly;(DB22-20:6H)the RF divider is 64;(DB19-12:C8H)R is 200
    buffer[1] = 0x80; //(DB11=0)VCO powerd up;
    buffer[0] = 0x3C; //(DB5=1)RF output is enabled;(DB4-3=3H)Output power level is +5dBm
    writeToADF4351(4, buffer);

    // Init Register R3:
    buffer[3] = 0x00;
    buffer[2] = 0x00; //(DB16-15:00)CLOCK DIVIDER OFF
    buffer[1] = 0x00; //(DB14-3:96H)clock divider value is 150.
    buffer[0] = 0x03;
    writeToADF4351(4, buffer);

    // Init Register R2:
    buffer[3] = 0x00;
    buffer[2] = 0x01; //(DB23-14:4H)R counter is 4
    buffer[1] = 0x0E; //(DB12-9:7H)set Icp 2.50 mA;
    buffer[0] = 0x42; //(DB8=0)enable fractional-N digital lock detect; (DB6=1)set PD polarity is positive;(DB7=0)LDP is 10ns;
    writeToADF4351(4, buffer);

    // Init Register R1:
    buffer[3] = 0x08; //(DB27=1)prescaler value is 8/9
    buffer[2] = 0x00; //(DB26-15:1H)PHASE word is 1,neither the phase resync
    buffer[1] = 0x80; //(DB14-3:6H)MOD counter is 6;
    buffer[0] = 0x09; // nor the spurious optimization functions are being used
    writeToADF4351(4, buffer);

    // Init Register R0:
    buffer[3] = 0x00; //(DB30-15:59H)INT value is 89;
    buffer[2] = 0x00;
    buffer[1] = 0x00; //(DB14-3:0H)FRAC value is 2;
    buffer[0] = 0x00;
    writeToADF4351(4, buffer);
}

void writeToADF4351(uint8_t count, uint8_t *buf)
{
    uint8_t valueToWrite = 0;
    uint8_t i = 0;
    uint8_t j = 0;

    ADF4351_CE(1);
    usleep(1);
    ADF4351_CLK(0);
    ADF4351_LE(0);
    usleep(1);

    DAT_OUT();
    for (i = count; i > 0; i--)
    {
        valueToWrite = *(buf + i - 1); // VD: Truyen byte cao nhat (byte 4) se la: 0 + 4 - 1 = 3 (Chi so 3 nhung byte 4)
        for (j = 0; j < 8; j++)
        {
        	// Truyen tung bit mot, ghi ra cac chan IO la trang thai bit tuong ung:
            if (0x80 == (valueToWrite & 0x80))
            {
                ADF4351_OUTPUT_DATA(1);
            }
            else
            {
                ADF4351_OUTPUT_DATA(0);
            }
            usleep(1);
            ADF4351_CLK(1);
            usleep(1);
            valueToWrite <<= 1;
            ADF4351_CLK(0);
        }
    }
    ADF4351_OUTPUT_DATA(0);
    usleep(1);
    ADF4351_LE(1);
    usleep(1);
    ADF4351_LE(0);
}

void writeOneRegToADF4351(uint32_t reg)
{
    uint8_t buf[4] = {0, 0, 0, 0};
    buf[3] = (uint8_t)((reg >> 24) & (0X000000FF));
    buf[2] = (uint8_t)((reg >> 16) & (0X000000FF));
    buf[1] = (uint8_t)((reg >> 8) & (0X000000FF));
    buf[0] = (uint8_t)((reg) & (0X000000FF));
    writeToADF4351(4, buf);
}

void ADF4351WriteFreq(float fre)
{
    uint16_t fre_temp, n_mul = 1, mul_core = 0;
    uint16_t int_fre, frac_temp, mod_temp, i;
    uint32_t w_adf4351_r0 = 0, w_adf4351_r1 = 0, w_adf4351_r4 = 0;
    float multiple = 0.0;

    if (fre < 35.0)
        fre = 35.0;
    if (fre > 4400.0)
        fre = 4400.0;
    mod_temp = 1000;
    fre = ((float)((uint32_t)(fre * 10))) / 10;

    fre_temp = fre;
    for (i = 0; i <= 6; i++)
    {
        if (((fre_temp * n_mul) >= 2199.9) && ((fre_temp * n_mul) <= 4400.1))
            break;
        mul_core++;
        n_mul = n_mul * 2;
    }

    multiple = (fre * n_mul) / 25; // 25: Phase detection frequency,
                                   // onboard 100M reference, divided by 4 in the register
                                   // to get 25M phase detection.
                                   // If the user changes to 80M reference input,
                                   // 25 needs to be changed to 20; 10M reference input,
                                   // 25 needs to be changed to 2.5, and so on. . .
    int_fre = (uint16_t)multiple;
    frac_temp = ((uint32_t)(multiple * 1000)) % 1000;
    while (((frac_temp % 5) == 0) && ((mod_temp % 5) == 0))
    {
        frac_temp = frac_temp / 5;
        mod_temp = mod_temp / 5;
    }
    while (((frac_temp % 2) == 0) && ((mod_temp % 2) == 0))
    {
        frac_temp = frac_temp / 2;
        mod_temp = mod_temp / 2;
    }

    mul_core %= 7;
    w_adf4351_r0 = (int_fre << 15) + (frac_temp << 3);
    w_adf4351_r1 = ADF4351_R1_BASE + (mod_temp << 3);
    w_adf4351_r4 = ADF4351WriteMaxOutputPower(ADF4351_R4_BASE, level_reg);;
    w_adf4351_r4 = w_adf4351_r4 + (mul_core << 20);

    //	writeOneRegToADF4351(ADF4351_PD_OFF); //ADF4351_RF_OFF
    //	writeOneRegToADF4351((uint32_t)(ADF4351_R4_OFF + (6<<20)));
    writeOneRegToADF4351(ADF4351_RF_OFF);
    writeOneRegToADF4351(w_adf4351_r0);
    writeOneRegToADF4351(w_adf4351_r1);
    writeOneRegToADF4351(w_adf4351_r4);

    //	writeOneRegToADF4351(ADF4351_PD_ON);
}

uint32_t ADF4351WriteMaxOutputPower(uint32_t config_r4, int8_t lev)
{
	int8_t bit1 = 0;
	int8_t bit0 = 0;
    if (lev == 0x00)
    {
        bit1 = 0;
        bit0 = 0;
    }
    else if (lev == 0x01)
    {
        bit1 = 0;
        bit0 = 1;
    }
    else if (lev == 0x10)
    {
        bit1 = 1;
        bit0 = 0;
    }
    else if (lev == 0x11)
    {
        bit1 = 1;
        bit0 = 1;
    }


    if (bit1)
    {
    	config_r4 |= (1 << 4); // Set bit b4
    }
    else
    {
        config_r4 &= ~(1 << 4); // Delete bit b4
    }

    if (bit0)
    {
    	config_r4 |= (1 << 3); // Set bit b3
    }
    else
    {
    	config_r4 &= ~(1 << 3); // Delete bit b3
    }
    return config_r4;
}
