/*
 * Liquid_Crystal.h
 *
 *  Holds functions for controlling a Liquid Crystal Display
 *      init_LCD         - Configure LCD for use
 *      clear_LCD        - Clears LCD
 *      home_LCD         - returns cursor to top left position
 *      send_Command_LCD - Sends a command to the LCD controller
 *      print_Char_LCD   - Sends character to display on LCD
 *      print_String_LCD - Sends string to display on LCD
 *      set_Cursor_LCD   - Sets cursor to desired column/row
 *      send_Nibble_LCD  - Sends a nibble of data (not to be called externally)
 *
 *
 *
 *  Dependencies:
 *      MSP.h
 *      Clocks.h
 *      Uses a full port for control
 *
 *  Errors:
 *
 *  Revisions:
 *      Apr 10,  2017 - Initial Creation
 *      Apr 13,  2017 - Commented and compiled
 *
 *  Authors: Drew Hartley, Jordan Jones
 */

#ifndef LIQUID_CRYSTAL_H_
#define LIQUID_CRYSTAL_H_

// Necessary included file for functionality
#include "Clocks.h"
#include <string.h>

#ifndef FCPU
#warning "No FCPU Defined Assuming 24MHz"
#define  FCPU FREQ_24_MHZ
#endif

// Port/Pin defines
#define LCD_DATA_PORT   P7
#define LCD_ENABLE_PIN  BIT7
#define LCD_RW_PIN      BIT5
#define LCD_RS_PIN      BIT6

// Command and configuration defines
#define LCD_TWO_LINE_5X8        0x28
#define LCD_CURSOR_MOVES_RIGHT  0x06
#define LCD_CLEAR               0x01
#define LCD_TURN_ON             0x0C
#define LCD_TURN_OFF            0x08
#define LCD_HOME_CURSOR         0x02
#define LCD_CLEAR               0x01

////////////////////////////////////////////////////////////////////////
//                      Communication Subroutine                      //
////////////////////////////////////////////////////////////////////////

/* send_Nibble_LCD()
 *  This sends commands to the LCD controller
 *
 * Parameters:
 *  data    - data to send, takes 4 MSB, 4 LSB ignored
 *  control - information for the control lines
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void send_Nibble_LCD(unsigned int data, unsigned int control){
    //Mask and shift to get desired format
    data    = (data>>4) & 0x0f;
    control = (control) & 0xf0;

    //Set data and control lines, then pulse enable
    LCD_DATA_PORT->OUT  = data|control;
    LCD_DATA_PORT->OUT |= LCD_ENABLE_PIN;
    LCD_DATA_PORT->OUT &= ~LCD_ENABLE_PIN;

    //Clear output
    LCD_DATA_PORT->OUT  = 0;
}


////////////////////////////////////////////////////////////////////////
//                        Control Subroutines                         //
////////////////////////////////////////////////////////////////////////

/* send_Command_LCD()
 *  This sends commands to the LCD controller
 *
 * Parameters:
 *  command - command, defined above
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void send_Command_LCD(unsigned int command){
    //Send command nibble by nibble
    send_Nibble_LCD(command    , 0);
    send_Nibble_LCD(command<<4 , 0);

    //Delay for LCD
    delay_ms(4,FCPU);
}


/* send_Char_Data()
 *  This sends data to be displayed
 *
 * Parameters:
 *  data - desired character
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void print_Char_LCD(unsigned int value){
    //Send data nibble by nibble
    send_Nibble_LCD(value   , LCD_RS_PIN);
    send_Nibble_LCD(value<<4, LCD_RS_PIN);

    //Delay for LCD
    delay_ms(1,FCPU);
}


/* print_String_LCD()
 *  This sends data to be displayed from a string
 *
 * Parameters:
 *  data - desired string
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void print_String_LCD(const char* data){
    int i, length;
    length = strlen(data);
    for(i = 0; i<length; i++){
        print_Char_LCD(data[i]);
    }
}

/*  init_LCD()
 *   This configures the LCD, normally called on boot
 *   The LCD will boot into 5x8, 2 line, 4 bit mode
 *
 *  Parameters:
 *   None
 *
 * Returns:
 *  Void
 *
 *  Errors:
 *   None Currently Apr 13, 2017
 */
void init_LCD(void){
    //Configure pins and wait for LCD to boot
    LCD_DATA_PORT->DIR|= 0xFF;
    delay_ms(35,FCPU);

    //Set system to 4 bit mode, delay for LCD
    send_Nibble_LCD(LCD_TWO_LINE_5X8,0);
    delay_ms(3,FCPU);

    //Turn off display in case it is already enabled
    send_Command_LCD(LCD_TURN_OFF);
    delay_ms(3,FCPU);

    //Setup display to desired configuration
    send_Command_LCD(LCD_TWO_LINE_5X8);
    send_Command_LCD(LCD_CURSOR_MOVES_RIGHT);
    send_Command_LCD(LCD_CLEAR);

    //Turn on LCD
    send_Command_LCD(LCD_TURN_ON);
    delay_ms(3,FCPU);

}


/* clear_LCD()
 *  This clears the LCD
 *  It is more of a wrapper for send_Command_LCD
 *
 * Parameters:
 *  None
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void clear_LCD(void){
   send_Command_LCD(LCD_CLEAR);
}


/* home_LCD()
 *  This sends the cursor to the top left corner
 *  It is more of a wrapper for send_Command_LCD
 *
 * Parameters:
 *  None
 *
 * Returns:
 *  Void
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
void home_LCD(void){
    send_Command_LCD(LCD_HOME_CURSOR);
}


/* set_cursor_LCD()
 *  This sets the cursor of the LCD to a set spot
 *
 * Parameters:
 *  column - value for column (indexed at 0)
 *  row    - value for row (indexed at 0) (default 0)
 *
 * Returns:
 *  0 - No error
 * -1 - Index out of bounds
 *
 * Errors:
 *  None Currently - Apr 13, 2017
 */
int set_Cursor_LCD(unsigned int column, unsigned int row){
    if(column > 0x27 || row > 1)
        return -1;//Kick out bad values

    //Get column value
    uint8_t address = column & 0x3F;

    //Add row value if needed
    if(row == 1)
        address += 0x40;
    //Add command bit to address
    address |= (0x80);
    //Send address to LCD
    send_Command_LCD(address);

    return 0;
}

#endif /* LIQUID_CRYSTAL_H_ */
