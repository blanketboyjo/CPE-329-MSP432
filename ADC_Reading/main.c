//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "Clocks.h"
#include "ADC.h"

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    set_DCO(FREQ_48_MHZ);
    __enable_irq();

    init_ADC();
    P1->DIR |= BIT0;
    run_ADC();
    while(1){
        if(hasNew_ADC()){
            if(1000 <= get_ADC()){
                P1->OUT |= BIT0;
            }else{
                P1->OUT &= ~BIT0;
            }
            run_ADC();
        }
    }
}


