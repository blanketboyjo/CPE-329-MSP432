/*
 *  ADC.C
 *    This holds the internal functions for UART and protected values
 *    See ADC.h for more details
 *
 * Errors:
 *   None Currently May 10, 2017
 *
 * Revisions:
 *   May 10,  2017 - Initial Creation
 *   May 12, 2017 - Cleaned and commented
 *
 *  Author: Drew Hartley, Jordan Jones
 */
#include "ADC.h"

#define CALIBRATION_VALUE .000201
static unsigned int ADCValue;
static int newValue;

void init_ADC(void){
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);         // Enable ADC interrupt in NVIC module
    ADC14->CTL0  = 0;      // Ensure it is off and all settings cleared
    ADC14->CTL0  = ADC14_CTL0_SHT0_2  | ADC14_CTL0_SHP |
                   ADC14_CTL0_PDIV__4 | ADC14_CTL0_MSC |
                   ADC14_CTL0_ON;

    ADC14->CTL1 = ADC14_CTL1_RES__14BIT;      // Use sampling timer, 12-bit conversion results
    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_1;     // A2 ADC input in single mode select; Vref=AVCC
    ADC14->IER0 |= ADC14_IER0_IE0;            // Enable ADC conv complete interrupt
    ADC14->CTL0 |= ADC14_CTL0_ENC;            // Enable ADC with current configuration
    P5->SEL1 |= BIT4;                         // Configure P5.4 for ADC
    P5->SEL0 |= BIT4;
}

void ADC14_IRQHandler(void){
    ADCValue = ADC14->MEM[0];                   // Store new data to buffer
    newValue = 1;                               // Set flag
}

int hasNew_ADC(void){
    return newValue;                            // Return flag value
}

unsigned int get_Raw_ADC(void){
    newValue = 0;                               // Clear flag
    return ADCValue;                            // Return raw unsigned int
}

float get_ADC(void){
    newValue = 0;                               // Clear flag
    return ((float)ADCValue)*CALIBRATION_VALUE; // Return calibrated value
}

void run_ADC(void){
    if(0 == (ADC14->CTL0 & ADC14_CTL0_SC)){     // If System not running
        ADC14->CTL0 |= ADC14_CTL0_SC;           // Start conversion
        newValue = 0;                           // Clear flag
    }
}


