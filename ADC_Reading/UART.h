/*
 * UART.h
 *
 *   This libary holds functions for UART, specifically for assignment 7
 *    init_UART         - Starts UART at a give baud
 *    print_Char_UART   - Prints a single char to the terminal
 *    print_String_UART - Prints a string to the terminal
 *    int_getDACValue   - Returns value of DAC
 *    int hasNewValue   - Returns whether there is a new value for DAC
 *
 * Depenedencies:
 *   MSP.h -  Needed for direct register access
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
 
#ifndef UART_UART_H_
#define UART_UART_H_
#include "MSP.h"
#include <string.h>
#define UART_BUFFER_LENGTH 20
#define F_CPU 48000000


void init_UART(unsigned int baud);
void print_Char_UART(char data);
void print_String_UART(const char* data);
int  getDACValue(void);
int  hasNewValue(void);

//Commented out useful for later implementations
//int  available_UART(void);
//char read_Char_UART(void);
//void flush_RX_UART(void);

#endif /* UART_UART_H_ */
