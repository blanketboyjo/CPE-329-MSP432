/*
 * Keypad.h
 *
 *  Created on: Apr 14, 2017
 *      Author: Jordan
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#ifndef FCPU
#warning "No FCPU Defined Assuming 24MHz"
#define  FCPU FREQ_24_MHZ
#endif

#define ROW_PORT    P10
#define ROW_PINS    0x0F
#define COLUMN_PORT P4
#define COLUMN_PINS 0x07

#define BUTTON_ONE          0x04
#define BUTTON_TWO          0x02
#define BUTTON_THREE        0x01
#define BUTTON_FOUR         0x20
#define BUTTON_FIVE         0x10
#define BUTTON_SIX          0x08
#define BUTTON_SEVEN        0x100
#define BUTTON_EIGHT        0x80
#define BUTTON_NINE         0x40
#define BUTTON_ASTERISK     0x800
#define BUTTON_ZERO         0x400
#define BUTTON_POUND        0x200


void init_Keypad(void){

    //Configure Row pins, output, low
    ROW_PORT->DIR     |=  (ROW_PINS);
    ROW_PORT->SEL0    &= ~(ROW_PINS);
    ROW_PORT->SEL1    &= ~(ROW_PINS);
    ROW_PORT->OUT     &= ~(ROW_PINS);
    ROW_PORT->REN     &= ~(ROW_PINS);


    //Configure Column pins, input, pullup
    COLUMN_PORT->DIR &= ~COLUMN_PINS;
    COLUMN_PORT->OUT |= COLUMN_PINS;
    COLUMN_PORT->REN |= COLUMN_PINS;
    COLUMN_PORT->IES |= COLUMN_PINS;
}

int get_Keypad(void){
    unsigned int row,output;
    output = 0;
    for(row = 0; row <4; row++){
        ROW_PORT->DIR &= ~(ROW_PINS);
        ROW_PORT->DIR |=  (1 <<row);
        delay_ns(1900,FCPU);
        output |= (((~COLUMN_PORT->IN) & COLUMN_PINS)<<(3*row));
    }
    delay_ms(1,FCPU);
    ROW_PORT->DIR     |=  (ROW_PINS);
    ROW_PORT->OUT     &= ~(ROW_PINS);
    return output;
}

char get_Keycode(void){
    static int lastValue = 0;
    int currentValue = get_Keypad();
    int newPresses = (currentValue ^ lastValue) & currentValue;
    lastValue = currentValue;
    if(newPresses & BUTTON_ASTERISK){
        return '*';
    }else if(newPresses & BUTTON_POUND){
        return '#';
    }else if(newPresses & BUTTON_ONE){
        return '1';
    }else if(newPresses & BUTTON_TWO){
        return '2';
    }else if(newPresses & BUTTON_THREE){
        return '3';
    }else if(newPresses & BUTTON_FOUR){
        return '4';
    }else if(newPresses & BUTTON_FIVE){
        return '5';
    }else if(newPresses & BUTTON_SIX){
        return '6';
    }else if(newPresses & BUTTON_SEVEN){
        return '7';
    }else if(newPresses & BUTTON_EIGHT){
        return '8';
    }else if(newPresses & BUTTON_NINE){
        return '9';
    }else if(newPresses & BUTTON_ZERO){
        return '0';
    }
    return 0;
}

void ie_Keypad(void){
    COLUMN_PORT->IFG &= ~COLUMN_PINS;//Clear flag before exit
    COLUMN_PORT->IE  |= COLUMN_PINS;
}

void id_Keypad(void){
    COLUMN_PORT->IE  &= ~COLUMN_PINS;
}


#endif /* KEYPAD_H_ */
