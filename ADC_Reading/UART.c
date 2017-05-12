/*
 *  UART.C
 *    This holds the internal functions for UART and protected values
 *    See UART.h for more details
 *  
 * Errors:
 *   None Currently May 12, 2017
 *
 * Revisions:
 *   May 5,  2017 - Initial Creation
 *   May 10, 2017 - Modified to work with assignment 8
 *   May 12, 2017 - Cleaned and commented
 *
 *  Author: Drew Hartley, Jordan Jones
 */
#include "UART.h"

static volatile char TX_BUFFER[ UART_BUFFER_LENGTH ];
static volatile unsigned int TX_WRITE_INDEX;
static volatile unsigned int TX_LOAD_INDEX;

void init_UART(unsigned int baud){

    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Reset USCI

    //Calculate float for divider, get first two decimal places
    float n = F_CPU/baud;
    int nFractional = (unsigned int)((n - (unsigned int) n)*100);

    if(n <=16){//No over-sampling see technical reference
        EUSCI_A0->MCTLW  = 0x00;
        EUSCI_A0->BRW = (unsigned int)n;

    }else{
        //Over-sampling required see technical reference
        EUSCI_A0->MCTLW  =  EUSCI_A_MCTLW_OS16;
        EUSCI_A0->BRW    = (unsigned int)(n/16);
        EUSCI_A0->MCTLW |= ((unsigned int)(((n/16) -((unsigned int)(n/16)))*16))
                                        << EUSCI_A_MCTLW_BRF_OFS;
    }
    //Fractional lookup table, no better way to implement
    if(5 >= nFractional)       EUSCI_A0->MCTLW |= 0x0000;
    else if( 7 >= nFractional) EUSCI_A0->MCTLW |= 0x0100;
    else if( 8 >= nFractional) EUSCI_A0->MCTLW |= 0x0200;
    else if(10 >= nFractional) EUSCI_A0->MCTLW |= 0x0400;
    else if(12 >= nFractional) EUSCI_A0->MCTLW |= 0x0800;
    else if(14 >= nFractional) EUSCI_A0->MCTLW |= 0x1000;
    else if(16 >= nFractional) EUSCI_A0->MCTLW |= 0x2000;
    else if(21 >= nFractional) EUSCI_A0->MCTLW |= 0x1100;
    else if(22 >= nFractional) EUSCI_A0->MCTLW |= 0x2100;
    else if(25 >= nFractional) EUSCI_A0->MCTLW |= 0x2200;
    else if(30 >= nFractional) EUSCI_A0->MCTLW |= 0x4400;
    else if(33 >= nFractional) EUSCI_A0->MCTLW |= 0x2500;
    else if(35 >= nFractional) EUSCI_A0->MCTLW |= 0x4900;
    else if(37 >= nFractional) EUSCI_A0->MCTLW |= 0x4A00;
    else if(40 >= nFractional) EUSCI_A0->MCTLW |= 0x5200;
    else if(42 >= nFractional) EUSCI_A0->MCTLW |= 0x9200;
    else if(43 >= nFractional) EUSCI_A0->MCTLW |= 0x5300;
    else if(50 >= nFractional) EUSCI_A0->MCTLW |= 0x5500;
    else if(57 >= nFractional) EUSCI_A0->MCTLW |= 0xAA00;
    else if(60 >= nFractional) EUSCI_A0->MCTLW |= 0x6B00;
    else if(62 >= nFractional) EUSCI_A0->MCTLW |= 0xAD00;
    else if(64 >= nFractional) EUSCI_A0->MCTLW |= 0xB500;
    else if(66 >= nFractional) EUSCI_A0->MCTLW |= 0xB600;
    else if(70 >= nFractional) EUSCI_A0->MCTLW |= 0xD600;
    else if(71 >= nFractional) EUSCI_A0->MCTLW |= 0xB700;
    else if(75 >= nFractional) EUSCI_A0->MCTLW |= 0xBB00;
    else if(78 >= nFractional) EUSCI_A0->MCTLW |= 0xDD00;
    else if(80 >= nFractional) EUSCI_A0->MCTLW |= 0xED00;
    else if(83 >= nFractional) EUSCI_A0->MCTLW |= 0xEE00;
    else if(84 >= nFractional) EUSCI_A0->MCTLW |= 0xBF00;
    else if(85 >= nFractional) EUSCI_A0->MCTLW |= 0xDF00;
    else if(87 >= nFractional) EUSCI_A0->MCTLW |= 0xEF00;
    else if(90 >= nFractional) EUSCI_A0->MCTLW |= 0xF700;
    else if(91 >= nFractional) EUSCI_A0->MCTLW |= 0xFB00;
    else if(92 >= nFractional) EUSCI_A0->MCTLW |= 0xFD00;
    else                       EUSCI_A0->MCTLW |= 0xFE00;

    //Select MCLK
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SSEL__SMCLK;

    //Configure pins
    P1->SEL0 |=  (BIT3 | BIT2);
    P1->SEL1 &= ~(BIT3 | BIT2);
    //End reset
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;

    //Enable interupts
    EUSCI_A0->IFG   = 0;
    EUSCI_A0->IE |= EUSCI_A_IE_TXCPTIE ;                    // Only enable TX complete interrupt
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);


}

void print_Char_UART(char data){
    int compareIndex  = TX_LOAD_INDEX == TX_WRITE_INDEX;    // Check if indexes are equal
    TX_BUFFER[TX_LOAD_INDEX] = data;                        // Load data to buffer
    TX_LOAD_INDEX = (TX_LOAD_INDEX + 1)% UART_BUFFER_LENGTH;// Increment buffer with rollover
    if(compareIndex){                                       // If index was equal (interrupt disabled)
        EUSCI_A0->TXBUF = TX_BUFFER[TX_WRITE_INDEX];        // Send char to UART buffer
        EUSCI_A0->IFG  &= ~EUSCI_A_IFG_TXCPTIFG;            // Clear interrupt flag
        EUSCI_A0->IE   |= EUSCI_A_IE_TXCPTIE;               // Enable TX complete interrupt
    }
}

void print_String_UART(const char* data){
    int i, length, compareIndex;
    length = strlen(data);                                  // Check length of the string
    compareIndex = TX_LOAD_INDEX == TX_WRITE_INDEX;         // Check if indexes are equal
    for(i = 0; i<length; i++){                              // Loop through full string
        TX_BUFFER[TX_LOAD_INDEX] = data[i];                 // Add char from string to buffer
        TX_LOAD_INDEX = (TX_LOAD_INDEX +1)                  // Increment index with rollover
                      % UART_BUFFER_LENGTH;
    }
    
    if(compareIndex){                                       // If index was equal (interrupt disabled)
        EUSCI_A0->TXBUF = TX_BUFFER[TX_WRITE_INDEX];        // Send a char to UART buffer
        EUSCI_A0->IFG  &= ~EUSCI_A_IFG_TXCPTIFG;            // Clear interrupt flag
        EUSCI_A0->IE   |= EUSCI_A_IE_TXCPTIE;               // Enable TX complete interrupt
    }
}

void EUSCIA0_IRQHandler(void){
    //////////////////////////////////////////////////////////
    //                  Receive Handler                     //
    //////////////////////////////////////////////////////////
//    if(EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG){                  // If flag is for RX
//
//    }

    //////////////////////////////////////////////////////////
    //                  Transmit Handler                    //
    //////////////////////////////////////////////////////////
    if(EUSCI_A0->IFG & EUSCI_A_IFG_TXCPTIFG){
        EUSCI_A0->IFG   &= ~EUSCI_A_IFG_TXCPTIFG;           // Clear flag
        TX_WRITE_INDEX = (1+ TX_WRITE_INDEX)                // Increment index with rollover 
                       % UART_BUFFER_LENGTH;
        if(TX_WRITE_INDEX != TX_LOAD_INDEX){                // If indexes are not the same
            EUSCI_A0->TXBUF= TX_BUFFER[TX_WRITE_INDEX];     // Load data to UART TX buffer
        }else{                                              // Else no more data
            EUSCI_A0->IE &= ~EUSCI_A_IE_TXCPTIE;            // Stop interrupt
        }
    }
}

int  transmission_Complete_UART(void){
    return  TX_WRITE_INDEX == TX_LOAD_INDEX;                // Return whether tx indices are equal
}
