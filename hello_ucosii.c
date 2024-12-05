#include <stdio.h>
#include <system.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "includes.h"
#include "altera_avalon_uart_regs.h"
#include "altera_modular_adc.h"
#include "sys/alt_irq.h"
#include "ADF4351.h"
#include "AD8318.h"
#include "SHT3X.h"
#include "TM1637.h"
#include "INA219_LO.h"

#define TASK_STACKSIZE 2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];
OS_STK task3_stk[TASK_STACKSIZE];
OS_STK task4_stk[TASK_STACKSIZE];
OS_STK task5_stk[TASK_STACKSIZE];
OS_STK task6_stk[TASK_STACKSIZE];
OS_STK task7_stk[TASK_STACKSIZE];

#define TASK1_PRIORITY 2
#define TASK2_PRIORITY 3
#define TASK3_PRIORITY 4
#define TASK4_PRIORITY 5
#define TASK5_PRIORITY 6
#define TASK6_PRIORITY 7
#define TASK7_PRIORITY 8

OS_EVENT *ResourceMutex;

#define RX_BUFFER_SIZE 500
char rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_index = 0;
uint8_t rx_data = 0;

volatile uint8_t adf4351_data_ready = 0;
volatile uint8_t max_output_power_data_ready = 0;
volatile uint8_t dutyCycle_data_ready = 0;
volatile uint8_t control_led_ready = 0;
volatile uint8_t response = 0;

int value_write_frequency = 0;
int8_t level_reg = OUTPUT_POWER_MAX_LV4;

alt_u32 adc_value;
double adc_distortion_voltage;
double adc_deviation;
double adc_voltage;

void IRQ_UART1_Interrupt(void);
void Clear_Buffer_End_Byte(void);
void IRQ_UART1_Init(void);
void Send_String_UART(const char *str);
void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
void Task4(void *pdata);
void Task5(void *pdata);
void Task6(void *pdata);
void Task7(void *pdata);

int main(void)
{
    IRQ_UART1_Init();
    // Configure the ADC for single measurements
    // must stop the ADC to change values
    adc_stop(ADC1_SEQUENCER_CSR_BASE); // ADC must be stopped to change the modes
    adc_set_mode_run_continuously(ADC1_SEQUENCER_CSR_BASE);
    adc_start(ADC1_SEQUENCER_CSR_BASE);
    INT8U err;
    OSInit();
    ResourceMutex = OSMutexCreate(1, &err);
    OSTaskCreateExt(Task1,
                    NULL,
                    (void *)&task1_stk[TASK_STACKSIZE - 1],
                    TASK1_PRIORITY,
                    TASK1_PRIORITY,
                    task1_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task2,
                    NULL,
                    (void *)&task2_stk[TASK_STACKSIZE - 1],
                    TASK2_PRIORITY,
                    TASK2_PRIORITY,
                    task2_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task3,
                    NULL,
                    (void *)&task3_stk[TASK_STACKSIZE - 1],
                    TASK3_PRIORITY,
                    TASK3_PRIORITY,
                    task3_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task4,
                    NULL,
                    (void *)&task4_stk[TASK_STACKSIZE - 1],
                    TASK4_PRIORITY,
                    TASK4_PRIORITY,
                    task4_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task5,
                    NULL,
                    (void *)&task5_stk[TASK_STACKSIZE - 1],
                    TASK5_PRIORITY,
                    TASK5_PRIORITY,
                    task5_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task6,
                    NULL,
                    (void *)&task6_stk[TASK_STACKSIZE - 1],
                    TASK6_PRIORITY,
                    TASK6_PRIORITY,
                    task6_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSTaskCreateExt(Task7,
                    NULL,
                    (void *)&task7_stk[TASK_STACKSIZE - 1],
                    TASK7_PRIORITY,
                    TASK7_PRIORITY,
                    task7_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);
    OSStart();
    return 0;
}

void IRQ_UART1_Init(void)
{
    IOWR_ALTERA_AVALON_UART_STATUS(UART1_BASE, 0);
    uint32_t control_reg = IORD_ALTERA_AVALON_UART_CONTROL(UART1_BASE);
    control_reg |= ALTERA_AVALON_UART_CONTROL_RRDY_MSK;
    control_reg &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
    IOWR_ALTERA_AVALON_UART_CONTROL(UART1_BASE, control_reg);
    alt_ic_isr_register(
        UART1_IRQ_INTERRUPT_CONTROLLER_ID,
        UART1_IRQ,
        (alt_isr_func)IRQ_UART1_Interrupt,
        0x0,
        0x0);
}

void IRQ_UART1_Interrupt(void)
{
    OSIntEnter();
    rx_data = IORD_ALTERA_AVALON_UART_RXDATA(UART1_BASE);
    if (rx_data != '\n')
    {
        if (rx_index < RX_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = rx_data;
        }
        else
        {
            Clear_Buffer_End_Byte();
        }
    }
    else
    {
        rx_buffer[rx_index] = '\0';
        if (strstr(rx_buffer, "RF_OUTPUT") != NULL)
        {
            adf4351_data_ready = 1;
        }
        else if (strstr(rx_buffer, "MAXIMUM_OUTPUT_POWER") != NULL)
        {
            max_output_power_data_ready = 1;
        }
        else if (strstr(rx_buffer, "DUTY_CYCLE") != NULL)
        {
            dutyCycle_data_ready = 1;
        }
        else if (strstr(rx_buffer, "HUMIDITY") != NULL)
        {
            control_led_ready = 1;
            //        	Send_String_UART("Response: LED 7 segment display: Humidity\n");
            response = 1;
        }
        else if (strstr(rx_buffer, "TEMPERATURE") != NULL)
        {
            control_led_ready = 0;
            response = 2;
            //        	Send_String_UART("Response: LED 7 segment display: Temperature\n");
        }
        else
        {
            response = 3;
            //            Send_String_UART("Response: String invalid!\n");
        }
    }
    OSIntExit();
}

void Clear_Buffer_End_Byte(void)
{
    memset(rx_buffer, 0, RX_BUFFER_SIZE);
    rx_index = 0;
}

void Send_String_UART(const char *str)
{
    while (*str)
    {
        while (!(IORD_ALTERA_AVALON_UART_STATUS(UART1_BASE) & ALTERA_AVALON_UART_STATUS_TRDY_MSK))
            ;
        IOWR_ALTERA_AVALON_UART_TXDATA(UART1_BASE, *str);
        str++;
    }
}

void Task1(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 1\n");

        if (adf4351_data_ready)
        {
            char *adf_ptr = strstr(rx_buffer, "RF_OUTPUT");
            if (adf_ptr != NULL)
            {
                char *ptr = adf_ptr + 9;
                while (*ptr == ' ')
                {
                    ptr++;
                }
                if (isdigit(*ptr))
                {
                	value_write_frequency = strtol(ptr, NULL, 10);
                    if (value_write_frequency >= 35 && value_write_frequency <= 4400)
                    {
                        Send_String_UART("Response: Detected RF output value: ");
                        char value_str[10];
                        sprintf(value_str, "%d MHz", value_write_frequency);
                        Send_String_UART(value_str);
                        Send_String_UART("\n");

                        ADF4351Init();
                        ADF4351WriteFreq(value_write_frequency);
                    }
                    else
                    {
                        Send_String_UART("Response: Invalid value for RF output\n");
                    }
                }
                else
                {
                    Send_String_UART("Response: Please enter numeric value\n");
                }
            }
            Clear_Buffer_End_Byte();
            adf4351_data_ready = 0;
        }

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}

void Task2(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 2\n");
        if (max_output_power_data_ready)
        {
            char *adf_ptr = strstr(rx_buffer, "MAXIMUM_OUTPUT_POWER");
            if (adf_ptr != NULL)
            {
                char *ptr = adf_ptr + 20;
                while (*ptr == ' ')
                {
                    ptr++;
                }
                if (*ptr == '-' || *ptr == '+')
                {
                    ptr++;
                }

                if (isdigit(*ptr))
                {
                	int value = strtol(ptr - 1, NULL, 10);
                    if (value == -4 || value == -1 || value == 2 || value == 5)
                    {
                        Send_String_UART("Response: Detected maximum output power value: ");
                        char value_str[10];
                        sprintf(value_str, "%d dBm", value);
                        Send_String_UART(value_str);
                        Send_String_UART("\n");

                        if (value == -4)
                        {
                        	level_reg = OUTPUT_POWER_MAX_LV1;
                        }
                        else if (value == -1)
                        {
                        	level_reg = OUTPUT_POWER_MAX_LV2;
                        }
                        else if (value == +2)
                        {
                        	level_reg = OUTPUT_POWER_MAX_LV3;
                        }
                        else
                        {
                        	level_reg = OUTPUT_POWER_MAX_LV4;
                        }
                        ADF4351Init();
                        ADF4351WriteFreq(value_write_frequency);
                    }
                    else
                    {
                        Send_String_UART("Response: Invalid value for maximum output power\n");
                    }
                }
                else
                {
                    Send_String_UART("Response: Please enter numeric value\n");
                }
            }
            Clear_Buffer_End_Byte();
            max_output_power_data_ready = 0;
        }
        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}

void Task3(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 3\n");

        if (dutyCycle_data_ready)
        {
            char *dutyCycle_ptr = strstr(rx_buffer, "DUTY_CYCLE");
            if (dutyCycle_ptr != NULL)
            {
                char *ptr = dutyCycle_ptr + 10;
                while (*ptr == ' ')
                {
                    ptr++;
                }
                if (isdigit(*ptr))
                {
                    int value = strtol(ptr, NULL, 10);
                    if ((value == 0) || (value == 10) || (value == 20) || (value == 30) || (value == 40) ||
                        (value == 50) || (value == 60) || (value == 70) || (value == 80) || (value == 90) ||
                        (value == 100))
                    {
                        Send_String_UART("Response: Detected duty cycle value: ");
                        char value_str[10];
                        sprintf(value_str, "%d %%", value);
                        Send_String_UART(value_str);
                        Send_String_UART("\n");

                        IOWR(PWM_BASE, 0, value);
                    }
                    else
                    {
                        Send_String_UART("Response: Invalid value for duty cycle\n");
                    }
                }
                else
                {
                    Send_String_UART("Response: Please enter numeric value\n");
                }
            }
            Clear_Buffer_End_Byte();
            dutyCycle_data_ready = 0;
        }

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}

void Task4(void *pdata)
{
    uint8_t status = 1;
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 4\n");

        if (response == 1)
        {
            Send_String_UART("Response: LED 7 segment display: Humidity\n");
            Clear_Buffer_End_Byte();
            response = 0;
        }
        else if (response == 2)
        {
            Send_String_UART("Response: LED 7 segment display: Temperature\n");
            Clear_Buffer_End_Byte();
            response = 0;
        }
        else if (response == 3)
        {
            Send_String_UART("Response: String invalid!\n");
            Clear_Buffer_End_Byte();
            response = 0;
        }
        status = SHT3x_Read(0xe000);
        if (status == 0)
        {
            if (control_led_ready)
            {
                TM1637_WriteNum_AddressAutoMode(0xc0, humidity * 100, 1);
            }
            else
            {
                TM1637_WriteNum_AddressAutoMode(0xc0, temperature * 100, 1);
            }

            char value_str[10];
            Send_String_UART("HUM: ");
            sprintf(value_str, "%.2f", humidity);
            Send_String_UART(value_str);
            Send_String_UART("\n");

            Send_String_UART("TEMP: ");
            sprintf(value_str, "%.2f", temperature);
            Send_String_UART(value_str);
            Send_String_UART("\n");
        }

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
}

void Task5(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 5\n");

        alt_adc_word_read(ADC1_SAMPLE_STORE_CSR_BASE, &adc_value, ADC1_SAMPLE_STORE_CSR_CSD_LENGTH);
        adc_distortion_voltage = (double)adc_value * 2.5 / 4096.0;
        adc_deviation = 1.25 - adc_distortion_voltage; // deviation = Voffset - Vdistortion
        adc_voltage = 1.25 + adc_deviation;

        char value_str[10];
        Send_String_UART("ADC_DISTORTION_VOLTAGE_V: ");
        sprintf(value_str, "%.4lf", adc_distortion_voltage);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        Send_String_UART("ADC_DEVIATION: ");
        sprintf(value_str, "%.4lf", adc_deviation);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        Send_String_UART("ADC_VOLTAGE_V: ");
        sprintf(value_str, "%.4lf", adc_voltage);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 20);
    }
}

void Task6(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 6\n");

        double slope = -0.05;
        double intercept = -4.6;
        double error = 0.0;
        double output_power = 0.0;

        if (level_reg == OUTPUT_POWER_MAX_LV1)
        {
        	intercept = -8.6;
        }
        else if (level_reg == OUTPUT_POWER_MAX_LV2)
        {
        	intercept = -7.5;
        }
        else if (level_reg == OUTPUT_POWER_MAX_LV3)
        {
        	intercept = -6.1;
        }

        uint8_t result = (value_write_frequency / 100) % 10;

        if (value_write_frequency >= 35 && value_write_frequency <= 1000)
        {
        	intercept = intercept - 3;
        }
        else if (value_write_frequency > 1000 && value_write_frequency <= 2000)
        {
        	intercept = intercept - 2;
        }

    	if (value_write_frequency >= 35 && value_write_frequency <= 40)
    	{
    		error = +1;
    	}
    	else if (value_write_frequency > 40 && value_write_frequency < 80)
		{
    		error = +0.5;
    	}
    	else if (value_write_frequency > 80 && value_write_frequency <= 100)
		{
    		error = -0.5;
    	}
    	else if (result >=0 && result <= 3)
		{
    		error = -0.5;
    	}
    	else if (result > 3 && result < 8)
		{
    		error = +0.5;
    	}
    	else if (result > 8)
		{
    		error = -0.5;
    	}


        AD8318_Init(slope, intercept, error);
        char value_str[10];
        Send_String_UART("SLOPE: ");
        sprintf(value_str, "%.4lf", slope);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        Send_String_UART("INTERCEPT: ");
        sprintf(value_str, "%.4lf", intercept);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        output_power = AD8318_Get_Output_Power(adc_voltage);
        Send_String_UART("OUTPUT_POWER: ");
        sprintf(value_str, "%.4lf", output_power);
        Send_String_UART(value_str);
        Send_String_UART("\n");

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 20);
    }
}

void Task7(void *pdata)
{
    INT8U err;
    while (1)
    {
        OSMutexPend(ResourceMutex, 0, &err);
        Send_String_UART("Hello from Task 7\n");

        if (INA219_LO_Begin() == true)
        {
            char value_str[10];
            Send_String_UART("LO_LOAD_VOLTAGE_V: ");
            sprintf(value_str, "%.2f", INA219_LO_GetBusVoltage_V() + (INA219_LO_GetShuntVoltage_MV() / 1000));
            Send_String_UART(value_str);
            Send_String_UART("\n");

            Send_String_UART("LO_CURRENT_MA: ");
            sprintf(value_str, "%.2f", INA219_LO_GetCurrent_MA());
            Send_String_UART(value_str);
            Send_String_UART("\n");

            Send_String_UART("LO_POWER_MW: ");
            sprintf(value_str, "%.2f", INA219_LO_GetPower_MW());
            Send_String_UART(value_str);
            Send_String_UART("\n");
        }
        else
        {
            Send_String_UART("Response: Failed to find INA219 chip for LO\n");
        }

        OSMutexPost(ResourceMutex);
        OSTimeDlyHMSM(0, 0, 0, 20);
    }
}
