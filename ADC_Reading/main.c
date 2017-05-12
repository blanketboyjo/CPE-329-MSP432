/*
 * Main.c
 *    Main function for assignment 8 ADC
 *
 * Errors:
 *   None Currently May 5, 2017
 *
 * Revisions:
 *   May 5,  2017 - Initial Creation
 *   May 12, 2017 - Cleaned and commented
 *
 *  Author: Drew Hartley, Jordan Jones
 */

#include "msp.h"
#include "Clocks.h"
#include "ADC.h"
#include "UART.h"


#define ONES 1
#define TENTHS 3
#define HUNDREDTHS 4

void main(void)
{
    char message[] = {0x0D,'0','.','0','0'};    // Message format
                                                // x.xx with carriage return to overwrite
    WDTCTL = WDTPW | WDTHOLD;                   // Stop watchdog timer
    set_DCO(FREQ_48_MHZ);                       // Set to 48 MHz
    __enable_irq();                             // Enable interrupts
    init_UART(750000);                          // Start UART
    init_ADC();                                 // Start ADC

    while(1){
        if(transmission_Complete_UART()){       // Message sent
            print_String_UART(message);         // Load next message
            run_ADC();                          // Start next poll while sending data
            while(!hasNew_ADC());               // Wait for new ADC data (comes faster than UART send)
            float value = get_ADC();
                                                //Convert number to string
            message[ONES]       = (int)value + '0';
            message[TENTHS]     = (int)(value * 10)%10 + '0';
            message[HUNDREDTHS] = (int)(value * 100)%10 + '0';
        }
    }
}


