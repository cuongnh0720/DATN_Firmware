#ifndef _AD8318_H_
#define _AD8318_H_

// Include statements
#include <stdio.h>
#include <stdint.h>

// Function prototypes
void AD8318_Init(double slope, double intercept, double error);
double AD8318_Get_Output_Power(double adc_voltage);

#endif
