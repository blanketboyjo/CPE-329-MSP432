/*
 * Assignment 7 UART
 *
 *   This assignment takes in a value 0-4095 over UART and outputs it to the
 *   MCP4921 when enter is pressed in the terminal. It also error checks the
 *   input automatically within the ISR.
 *
 * Depenedencies:
 *   Clocks.h       - Holds functions for delays and cpu frequency changes
 *   SPI.h          - Holds functions for configuring and using SPI
 *   DAC.h          - Holds functions for DAC control
 *   UART.h         - Holds functions for UART communication
 *
 * Errors:
 *   None Currently May 5, 2017
 *
 * Revisions:
 *   May 5,  2017 - Initial Creation
 *
 *  Author: Drew Hartley, Jordan Jones
 *
 */

#include "msp.h"
#include "Clocks.h"
#include "UART.h"
#include "SPI.h"
#include "DAC.h"

void main(void)
{
	
    WDTCTL = WDTPW | WDTHOLD;             // Stop watchdog timer
    set_DCO(FREQ_48_MHZ);                 // Set to 48MHz
    init_SPI();                           // Start SPI
    init_DAC();                           // Startup DAC
    init_UART(9600);                      // Start UART at 9600 Baud
    __enable_irq();                       // Enable Global Interrupts


    while(1){
        if(hasNewValue()){                // New DAC value ready
            int newValue = getDACValue(); // Get new value
            send_DAC(newValue);           // Send new value to DAC
        }
    }
}

