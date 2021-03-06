/*
 * Waveforms.h
 *
 *  Created on: May 2, 2017
 *      Author: Jordan
 */

#ifndef WAVEFORMS_H_
#define WAVEFORMS_H_
#include "Liquid_Crystal.h"

//Enum for waveform for easier configurations
typedef enum{
    square,
    saw,
    sine
}Waveform;

//Data struct for complete wavedata
typedef struct{
    Waveform form;
    unsigned int      freq;
    unsigned int      duty;
}WaveData;

//Prints Waveform Data
void print_Wave_Data(WaveData input){
    //Character arrays for string data
    char *waveType;
    char duty[3];
    char freq[4];

    //Set LCD Cursor to 0,0 for data writing
    set_Cursor_LCD(0,0);

    //Convert frequency to characters and print to LCD
    sprintf(freq,"%d",input.freq);
    print_String_LCD(freq);
    print_String_LCD("Hz ");

    //Convert waveform to characters
    switch(input.form){
    case square:
        waveType = "Sqr ";
        break;

    case saw:
        waveType = "Saw ";
        break;

    case sine:
        waveType = "Sin ";
        break;
    }
    print_String_LCD(waveType);

    //Add duty cycle if needed
    if(square == input.form){
        sprintf(duty, "%d", input.duty);
        print_String_LCD(duty);
        print_Char_LCD('%');
    }else{
        print_String_LCD("       ");//Not cleanest way to clear row, but it works
    }
}



#endif /* WAVEFORMS_H_ */
