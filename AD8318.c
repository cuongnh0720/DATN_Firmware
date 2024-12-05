#include "AD8318.h"

static double _slope;
static double _intercept;
static double _error;

void AD8318_Init(double slope, double intercept, double error)
{
    _slope = slope;
    _intercept = intercept;
    _error = error;
}

double AD8318_Get_Output_Power(double adc_voltage)
{
    double output_power;

    output_power = (adc_voltage / _slope) + _intercept + 30.0 + _error;

    return output_power;
}

