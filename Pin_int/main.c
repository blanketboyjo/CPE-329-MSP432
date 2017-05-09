//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#define ROW_PORT    P10
#define ROW_PINS    0x0F
#define COLUMN_PORT P4
#define COLUMN_PINS 0x07

void main(void)
{
	ROW_PORT->DIR |= ROW_PINS;
	ROW_PORT->OUT &= ~ROW_PINS;
	COLUMN_PORT->DIR &= ~COLUMN_PINS;
	COLUMN_PORT->OUT |= COLUMN_PINS;
	COLUMN_PORT->REN |= COLUMN_PINS;
	COLUMN_PORT->IES |= COLUMN_PINS;
	__enable_irq();

	NVIC->ISER[1] = 1 << (PORT4_IRQn & 31);
	COLUMN_PORT->IE  |= COLUMN_PINS;
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
	while(1){

	}
}

void PORT4_IRQHandler(void){
    COLUMN_PORT->IFG &= ~COLUMN_PINS;
}
