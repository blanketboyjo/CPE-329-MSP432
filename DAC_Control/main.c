/*
 * Assignment 5 DAC waveform Generation
 *   This project can output a triangle or square wave at 50Hz
 *   with 2Vpp and 1 V offset.
 * 
 * Depenedencies:
 *   Clocks.h - Holds functions for delays and cpu frequency changes
 *   SPI.h    - Holds functions for configuring and using SPI
 *   Uses P9  - SPI pins
 *           P9.4 - CE   - Active low chip select
 *           P9.7 - SCLK - SPI Clock
 *           P9.5 - MOSI - SPI Master out
 *   
 * Errors:
 *   None Currently Apr 24, 2017
 *
 * Revisions:
 *   Apr 24,  2017 - Initial Creation
 *
 *  Author: Drew Hartley, Jordan Jones
 *
 */

///////////////////////////////////////////////////////////////////////
//                        Depenendency Includes                      //
///////////////////////////////////////////////////////////////////////
#include "msp.h"
#include "Clocks.h"
#include "SPI.h"

///////////////////////////////////////////////////////////////////////
//                        NOT TO BE EDITTED DEFINES                  //
///////////////////////////////////////////////////////////////////////
#define TRIANGLE_WAVE 1
#define SQUARE_WAVE   0

#define TWO_VOLT   1242
#define ZERO_VOLT  0

#define COUNT_SQUARE   30002
#define COUNT_TRIANGLE   193

///////////////////////////////////////////////////////////////////////
//                             Editiable Defines                     //
///////////////////////////////////////////////////////////////////////

//Change for different waveform
#define MODE TRIANGLE_WAVE

//Change to change chip select
#define CE_PIN  BIT4
#define CE_PORT P9

void send_DAC(unsigned int value);
void configure_Timer(void);

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    
    //Setup MCLK at 24 MHz
    set_DCO(FREQ_24_MHZ);

    //Setup SPI
    init_SPI();

    //Setup Chip select pin
    CE_PORT->SEL0 &= ~(CE_PIN);
    CE_PORT->SEL1 &= ~(CE_PIN);
    CE_PORT->DIR |= CE_PIN;
    CE_PORT->OUT |= CE_PIN;

    //Set DAC to start from zero
    send_DAC(ZERO_VOLT);

    configure_Timer();
    //Enab;e sleep on exit from isr
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    // Enable global interrupt
    __enable_irq();

    //Enable Timer Interrupt
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

    while(1){
        //Sleep, waiting for timer compare
        __sleep();
        __no_operation();
    }
}


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


///////////////////////////////////////////////////////////////////////
//                   Used In Square Wave Generation                  //
///////////////////////////////////////////////////////////////////////
#if MODE == SQUARE_WAVE
void configure_Timer(void){
    TIMER_A0->CCTL[0]   = TIMER_A_CCTLN_CCIE;         // Enable Compare Interrupt
    TIMER_A0->CCR[0]    = COUNT_SQUARE;               // Add needed offset
    TIMER_A0->CTL       = TIMER_A_CTL_SSEL__SMCLK |   // SMCLK source
                          TIMER_A_CTL_ID__8       |   // Divide by 8
                          TIMER_A_CTL_MC__CONTINUOUS; // Continuous Mode
}

void TA0_0_IRQHandler(void){
    static unsigned int currentValue = 0;             // Value for toggling
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;        // Clear Flag
    TIMER_A0->CCR[0]  +=  COUNT_SQUARE;               // Add offset
    
    if(currentValue == 0){                            // Low to High
        send_DAC(TWO_VOLT);                           // Send 2 Volts
        currentValue = 1;                             
    }else{
        send_DAC(ZERO_VOLT);                          // High to Low
        currentValue = 0;                             // Send 0 Volts
    }
}
#endif

///////////////////////////////////////////////////////////////////////
//                   Used In Triangle Wave Generation                //
///////////////////////////////////////////////////////////////////////
#if MODE == TRIANGLE_WAVE
void configure_Timer(void){
    TIMER_A0->CCTL[0]   = TIMER_A_CCTLN_CCIE;         // Enable Compare Interrupt
    TIMER_A0->CCR[0]    = COUNT_TRIANGLE;             // Add needed offset
    TIMER_A0->CTL       = TIMER_A_CTL_SSEL__SMCLK |   // SMCLK source
                          TIMER_A_CTL_MC__CONTINUOUS; // Continous mode
}

void TA0_0_IRQHandler(void){
    static unsigned int value = 0;                    // Value to send to DAC
    static int addValue = 1;                          // Value for changing DAC
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;        // Clear Flag
    TIMER_A0->CCR[0]  +=  COUNT_TRIANGLE;             // Add offset
    
    value += addValue;                                // Increment value
    send_DAC(value);                                  // Send value to DAC
    
    if(value == TWO_VOLT||value == ZERO_VOLT)         //If at limit flip addValue
        addValue = -addValue;
}
#endif


