/*
 * ADC.h
 *
 *   This libary holds functions for ADC, specifically for assignment 8
 *      init_ADC    - Configures ADC
 *      hasNew_ADC  - Returns whether ADC has a new value
 *      get_Raw_ADC - Returns direct value from ADC
 *      get_ADC     - Returns value in volts, 3.3 max
 *      run_ADC     - Starts another conversion
 *
 *
 * Depenedencies:
 *   MSP.h -  Needed for direct register access
 *
 * Errors:
 *   None Currently May 10, 2017
 *
 * Revisions:
 *   May 10, 2017 - Initial Creation
 *   May 12, 2017 - Cleaned and commented
 *
 *  Author: Drew Hartley, Jordan Jones
 *
 */

#ifndef ADC_H_
#define ADC_H_
#include "msp.h"


void init_ADC(void);
int hasNew_ADC(void);
unsigned int get_Raw_ADC(void);
float get_ADC(void);
void run_ADC(void);



#endif /* ADC_H_ */
