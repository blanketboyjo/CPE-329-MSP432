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
#include "stdio.h"
#include "Clocks.h"
#define FCPU FREQ_48_MHZ
#include "Liquid_Crystal.h"
#include "Keypad.h"
#include "SPI.h"
#include "DAC.h"
#include "Waveforms.h"

///////////////////////////////////////////////////////////////////////
//                        NOT TO BE EDITTED DEFINES                  //
///////////////////////////////////////////////////////////////////////
#define MAX_VAL_DAC         2048
#define MIN_VAL_DAC         0
#define MID_VAL_DAC         1024
#define SQUARE_RAW_COUNTS   6000000
#define SAW_RAW_COUNTS      48000000
#define SAW_RESOLUTION      2
#define SIN_ONE_HZ_COUNTS   160000

///////////////////////////////////////////////////////////////////////
//                             Editiable Defines                     //
///////////////////////////////////////////////////////////////////////

//Change for different waveform
#define MODE SQUARE_WAVE

void configure_Timer_Square(void);
void configure_Timer_Saw(void);
void configure_Timer_Sin(void);

void print_Wave_Data(WaveData input);
void ISR_Square(unsigned int * input, unsigned int * value);
void set_ISR_Configuration(WaveData input);

const unsigned int sinOffset[] ={
                                 0,    6,  13,  19,  26,  32,  39,  45,
                                51,   58,  64,  70,  77,  83,  89,  95,
                                101, 107, 113, 119, 125, 131, 137, 143,
                                148, 154, 160, 165, 170, 176, 181, 186,
                                191, 196, 201, 206, 211, 216, 220, 225,
                                229, 233, 237, 241, 245, 249, 253, 257,
                                260, 263, 267, 270, 273, 276, 279, 281,
                                284, 286, 289, 291, 293, 295, 297, 298,
                                300, 301, 303, 304, 305, 306, 306, 307,
                                307, 308, 308, 308,};

//Enum for state machine states
typedef enum{
    output,
    polling,
    updateLCD,
    prepare
}FSM_State;

typedef struct{
    unsigned int delay[2];
    unsigned int value[2];
}ISRConfig;

FSM_State volatile currentState = prepare;
WaveData  currentWave = {square, 100, 50};
ISRConfig ISRData = {{15000,15000},{MAX_VAL_DAC,MIN_VAL_DAC}};

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    //Setup MCLK at 48 MHz
    set_DCO(FREQ_48_MHZ);
    P1->DIR |= BIT0;
    P1->OUT &= ~BIT0;

    //Setup SPI, LCD, Keypad, DAC
    init_SPI();
    init_LCD();
    init_Keypad();
    init_DAC();

    //Set DAC to start from DC offset
    send_DAC(MID_VAL_DAC);

    //Configure all timers A0-A2
    configure_Timer_Square();
    configure_Timer_Saw();
    configure_Timer_Sin();

    //Enable Keypad Interrupts
    ie_Keypad();

    //Enable Global Interrupts
    __enable_irq();

    //Enable Timer Compare and Keypad Interrupts at NVIC
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
    NVIC->ISER[0] = 1 << ((TA1_0_IRQn) & 31);
    NVIC->ISER[0] = 1 << ((TA2_0_IRQn) & 31);
    NVIC->ISER[1] = 1 << ((PORT4_IRQn) & 31);

    while(1){//Run continously
        if(prepare == currentState){
            //Print wave data to screen
            print_Wave_Data(currentWave);
            set_Cursor_LCD(0,1);
            //Let user know system is outputing a waveform
            print_String_LCD("Enabled         ");

            //Call function for to load ISR configuration
            set_ISR_Configuration(currentWave);
            ie_Keypad();

        }else if (polling == currentState){
            char key = get_Keycode();       //Poll keypad for new key
            if(key != 0){                   //If key pressed modify waveform accordingly
                switch(key){
                case '1':
                    currentWave.freq = 100;
                    break;
                case '2':
                    currentWave.freq = 200;
                    break;
                case '3':
                    currentWave.freq = 300;
                    break;
                case '4':
                    currentWave.freq = 400;
                    break;
                case '5':
                    currentWave.freq = 500;
                    break;
                case '6':
                    currentState = prepare;
                    break;
                case '7':
                    currentWave.form = square;
                    break;
                case '8':
                    currentWave.form = saw;
                    break;
                case '9':
                    currentWave.form = sine;
                    break;
                case '0':
                    currentWave.duty = 50;
                    break;
                case '*':
                    if(currentWave.duty > 10)
                        currentWave.duty -= 10;
                    break;
                case '#':
                    if(currentWave.duty <90)
                        currentWave.duty += 10;
                    break;
                }

                //Update LCD with the new waveform information
                print_Wave_Data(currentWave);
            }
        }else if (updateLCD == currentState){
            set_Cursor_LCD(0,1);
            print_String_LCD("Disabled Press 6");
            currentState = polling;
        }
    }
}

void set_ISR_Configuration(WaveData input){
    //Disable all timer interrupts, redundant with port4 ISR but doesn't hurt to have
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;

    // Square wave setup
    if(square == input.form){
        unsigned int tenthDelay = SQUARE_RAW_COUNTS/        // Math for 10% dt
                (input.freq*100);
        ISRData.delay[0] = tenthDelay*input.duty;           // High Delay Multiply for given duty
        ISRData.delay[1] = tenthDelay*(100-input.duty);     // Low Delay Remainder

        TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;

    // Saw wave setup
    }else if(saw  == input.form){//If sawtooth
        ISRData.delay[0] = SAW_RAW_COUNTS/
                (input.freq*(MAX_VAL_DAC-MIN_VAL_DAC)/SAW_RESOLUTION);
        TIMER_A1->CCTL[0] = TIMER_A_CCTLN_CCIE;

    // Sine wave setup
    }else if(sine == input.form){
        ISRData.delay[0] = SIN_ONE_HZ_COUNTS/input.freq;
        TIMER_A2->CCTL[0] = TIMER_A_CCTLN_CCIE;
    }
    currentState = output;
}

///////////////////////////////////////////////////////////////////////
//                                     //
///////////////////////////////////////////////////////////////////////

void configure_Timer_Square(void){
    TIMER_A0->CCR[0]    = 255;                          // Add needed offset
    TIMER_A0->CTL       = TIMER_A_CTL_SSEL__SMCLK |     // SMCLK source
                          TIMER_A_CTL_ID__8       |     // Divide by 8
                          TIMER_A_CTL_MC__CONTINUOUS;   // Continuous Mode
}

void configure_Timer_Saw(void){
    TIMER_A1->CCR[0]    = 255;                          // Add needed offset
    TIMER_A1->CTL       = TIMER_A_CTL_SSEL__SMCLK |     // SMCLK source
                          TIMER_A_CTL_MC__CONTINUOUS;   // Continuous Mode
}

void configure_Timer_Sin(void){
    TIMER_A2->CCR[0]    = 255;                          // Add needed offset
    TIMER_A2->CTL       = TIMER_A_CTL_SSEL__SMCLK |     // SMCLK source
                          TIMER_A_CTL_MC__CONTINUOUS;   // Continuous Mode
}

void TA0_0_IRQHandler(void){
static unsigned int index = 0;                      //Index for arrays
    P1->OUT |= BIT0;
    //Send most recent value to DAC
    send_DAC(ISRData.value[index]);
    P1->OUT &= ~BIT0;
    //Offset compare value of timer
    TIMER_A0->CCR[0]  +=  ISRData.delay[index];

    // Toggle index
    index ^= 1;

    //Clear flag on exit
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

void TA1_0_IRQHandler(void){
    static unsigned int currentValue = MIN_VAL_DAC; // Value for DAC

    //Send most recent value to DAC
    send_DAC(currentValue);

    //Increment value for next ISR
    currentValue +=2;

    //If greater than max value then rest back to minumum
//    if(currentValue > MAX_VAL_DAC)
//        currentValue = MIN_VAL_DAC;

    //Increment offset
    TIMER_A1->CCR[0] += ISRData.delay[0];

    //Clear flag on exit
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

void TA2_0_IRQHandler(void){
    static int currentValue = MID_VAL_DAC;  // Value for toggling
    static int incrementIndex = 1;          // Value for incrementing index
    static int countState = 0;              // Value for deciding if count up or down
    static int index = 0;                   // Current Index

    //Upload most recent value to DAC
    send_DAC(currentValue);

    //Calculate next value for next ISR
    if(countState <2){
        currentValue = MID_VAL_DAC + sinOffset[index];
    }else{
        currentValue = MID_VAL_DAC - sinOffset[index];
    }

    //Increment Array Index
    index += incrementIndex;

    //Flip index incrementation if at min/max index
    if(index == 75|| index == 0){
        incrementIndex = -incrementIndex;
        countState =(countState + 1) % 4;
    }

    //Add offset to compare register
    TIMER_A2->CCR[0] += ISRData.delay[0];

    // Clear Flag
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

void PORT4_IRQHandler(void){
    //Stop all interrupts
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    id_Keypad();

    //Set state machine to disabled state
    currentState = updateLCD;

    //Clear flag before exit
    COLUMN_PORT->IFG &= ~COLUMN_PINS;
}
