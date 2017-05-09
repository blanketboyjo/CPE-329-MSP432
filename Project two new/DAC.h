/*
 * DAC.h
 *
 * This holds functions for controlling an MDP4921 DAC
 *  init_DAC - Configures DAC pins, starts SPI if necessary
 *  send_DAC - Sends a value to DAC to output
 *
 * Dependencies:
 *  MSP.h
 *  SPI.h
 *
 * Errors:
 *  None Currently May 3, 2017
 *
 * Revisions:
 *  Apr 24, 2017 - Initial Creation
 *  May  3, 2017 - init_DAC added
 *
 * Authors: Drew Hartley, Jordan Jones
 */

#ifndef DAC_H_
#define DAC_H_
#include "SPI.h"

//Change to change chip select
#define CE_PIN  BIT4
#define CE_PORT P9

/* init_DAC()
 *  This function configures the CE pin and possibly SPI if needed
 *
 * Parameters:
 *  none
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
void init_DAC(){
    //Setup Chip select pin
    CE_PORT->SEL0 &= ~(CE_PIN);
    CE_PORT->SEL1 &= ~(CE_PIN);
    CE_PORT->DIR |= CE_PIN;
    CE_PORT->OUT |= CE_PIN;

    //Init SPI if it is not already started
    init_SPI();
}


/* send_DAC()
 *  This function sends a value to the DAC to output in 2x mode
 *
 * Parameters:
 *  value - value for DAC, truncated to 12bit in function
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
void send_DAC(unsigned int value){
    unsigned int DAC_Word = 0;
    //Setup message, active mode, 2x Mode
    DAC_Word = (0x1000)|(value & 0x0FFF);
    //Send Chip select low, starting SPI communication
    CE_PORT->OUT &= ~(CE_PIN);
    //Send first byte
    send_SPI(DAC_Word >> 8);
    //Send second byte
    send_SPI(DAC_Word & 0x0FF);
    //Send Chip Select high, ending SPI communication
    CE_PORT->OUT |=  (CE_PIN);
}


#endif /* DAC_H_ */
