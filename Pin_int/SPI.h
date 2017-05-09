/*
 * SPI.h
 *  This holds functions for using SPI on EUSCI_A3
 *      init_SPI - Starts SPI on EUSCI_A3
 *      send_SPI - Sends one byte of data over SPI
 *
 *  Dependencies:
 *      MSP.h
 *      External Chip Select
 *      P9 Pins
 *          9.7 - MOSI
 *          9.5 - SCLK
 *
 *  Created on: Apr 24, 2017
 *      Author: Jordan
 */

#ifndef SPI_H_
#define SPI_H_

/* init_SPI()
 * Configures SPI on EUSCI_A3
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - Apr 24, 2017
 */
 
void init_SPI(void){

    //Prepare SPI pins
    P9->SEL0 |=   BIT7| BIT5;
    P9->SEL1 &= ~(BIT7| BIT5);

    //Set SPI to reset state to change values
    EUSCI_A3->CTLW0 |=  EUSCI_A_CTLW0_SWRST;
    EUSCI_A3->CTLW0  =  EUSCI_A_CTLW0_SWRST |      // Keep reset
                        EUSCI_A_CTLW0_SYNC  |      // Synchronous
                        EUSCI_A_CTLW0_MSB   |      // MSB first
                        EUSCI_A_CTLW0_CKPL  |      // High priority
                        EUSCI_A_CTLW0_MST;         // Master

    EUSCI_A3->CTLW0 |=  EUSCI_B_CTLW0_SSEL__SMCLK; // SMCLK as source
    EUSCI_A3->BRW    =  0x01;                      // No division
    EUSCI_A3->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;       // Stop reset state
    EUSCI_A3->IFG   |=  EUSCI_A_IFG_TXIFG;         // Do things
}


/* send_SPI()
 * Sends a byte of SPI, requires external chip select
 *
 * Parameters:
 *  data - data to send (only sends 8 LSB)
 *
 * Errors:
 *  None Currently - Apr 24, 2017
 */
void send_SPI(unsigned int data){
    EUSCI_A3->TXBUF = (unsigned char) data;        // Load Data into tx buffer
    while(!(EUSCI_A3->IFG & EUSCI_A_IFG_TXIFG));   // Wait for data to send
}



#endif /* SPI_H_ */
