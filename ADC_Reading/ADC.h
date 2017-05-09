/*
 * ADC.h
 *
 *  Created on: May 8, 2017
 *      Author: Jordan
 */

#ifndef ADC_H_
#define ADC_H_
#include "msp.h"


void init_ADC(void);
int hasNew_ADC(void);
unsigned int get_ADC(void);
void run_ADC(void);



#endif /* ADC_H_ */
