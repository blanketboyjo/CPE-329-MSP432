/*
 * UART.h
 *
 *   This libary holds functions for UART, specifically for assignment 8
 *    init_UART         - Starts UART at a give baud
 *    print_Char_UART   - Prints a single char to the terminal
 *    print_String_UART - Prints a string to the terminal
 *    transmission_Complete_UART - Returns if buffer is empty
 *
 * Depenedencies:
 *   MSP.h -  Needed for direct register access
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
int  transmission_Complete_UART(void);

#endif /* UART_UART_H_ */
