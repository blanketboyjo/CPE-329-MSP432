//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

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
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    set_DCO(FREQ_48_MHZ);
    __enable_irq();
    init_UART(750000);
    init_ADC();

    while(1){
        if(transmission_Complete_UART()){
            print_String_UART(message);
            run_ADC();                  // Start next poll while sending data
            while(!hasNew_ADC());         // Wait for new ADC data (comes faster than UART send)
            float value = get_ADC();
            //Convert number to string
            message[ONES]       = (int)value + '0';
            message[TENTHS]     = (int)(value * 10)%10 + '0';
            message[HUNDREDTHS] = (int)(value * 100)%10 + '0';
        }
    }
}


