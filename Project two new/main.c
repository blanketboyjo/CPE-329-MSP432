/*
 * Project 2 DAC waveform Generation
 *
 *   This project can output a triangle, square, or sine wave
 *   at 100Hz, 200Hz, 300Hz, 400Hz, and 500Hz with 5Vpp and 
 *   2.5 V offset. Duty cycle on square wave 10-90 in 10% steps
 *
 * Depenedencies:
 *   Clocks.h       - Holds functions for delays and cpu frequency changes
 *   SPI.h          - Holds functions for configuring and using SPI
 *   DAC.h          - Holds functions for DAC control
 *   Waveform.h     - Holds datatypes and functions for waveform configuration
 *   Liquid_Crystal - Holds functions for LCD
 *   Keypad         - Holds functions for using keypad
 *
 * Errors:
 *   None Currently May 3, 2017
 *
 * Revisions:
 *   May 3,  2017 - Initial Creation
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
#define SAW_RESOLUTION      8
#define SIN_ONE_HZ_COUNTS   160000

///////////////////////////////////////////////////////////////////////
//                             Type Defines                          //
///////////////////////////////////////////////////////////////////////

//Enum for state machine states
typedef enum{
    output,
    polling,
    disable,
    prepare
}FSM_State;


//Struct for ISR configuration
typedef struct{
    unsigned int delay[2]; // Delay times [1] only used in SQR 
    unsigned int value[2]; // DAC values, only used in SQR
}ISRConfig;

///////////////////////////////////////////////////////////////////////
//                             Function Names                        //
///////////////////////////////////////////////////////////////////////
void configure_Timer_Square(void);
void configure_Timer_Saw(void);
void configure_Timer_Sin(void);
void print_Wave_Data(WaveData input);
void ISR_Square(unsigned int * input, unsigned int * value);
void set_ISR_Configuration(WaveData input);


///////////////////////////////////////////////////////////////////////
//                              Global Data                          //
///////////////////////////////////////////////////////////////////////
const unsigned int sinOffset[] ={ // Sine wave lookup table
             0,   21,   43,   64,   86,  107,  128,  150,
           171,  192,  213,  234,  255,  275,  296,  316,
           337,  357,  377,  397,  417,  436,  455,  474,
           493,  512,  530,  549,  567,  584,  602,  619,
           636,  653,  669,  685,  701,  716,  732,  747,
           761,  775,  789,  803,  816,  828,  841,  853,
           865,  876,  887,  897,  908,  917,  927,  936,
           944,  952,  960,  967,  974,  980,  986,  992,
           997, 1002, 1006, 1010, 1013, 1016, 1018, 1020,
          1022, 1023, 1024, 1024};
          
//Global variables/structs for state control and ISR configuration
FSM_State volatile currentState = prepare;
WaveData            currentWave = {square, 100, 50};
ISRConfig ISRData = {{15000,15000},{MAX_VAL_DAC,MIN_VAL_DAC}};

///////////////////////////////////////////////////////////////////////
//                              Main function                        //
///////////////////////////////////////////////////////////////////////
void main(void){
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    set_DCO(FREQ_48_MHZ);           // Setup MCLK at 48 MHz

    //Setup SPI, LCD, Keypad, DAC
    init_SPI();
    init_LCD();
    init_Keypad();
    init_DAC();

    send_DAC(MID_VAL_DAC);          // Set DAC to start from DC offset

    // Configure all timers A0-A2
    configure_Timer_Square();
    configure_Timer_Saw();
    configure_Timer_Sin();

    ie_Keypad();                    // Enable Keypad Interrupts
    __enable_irq();                 // Enable Global Interrupts

    //Enable Timer Compare and Keypad Interrupts at NVIC
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
    NVIC->ISER[0] = 1 << ((TA1_0_IRQn) & 31);
    NVIC->ISER[0] = 1 << ((TA2_0_IRQn) & 31);
    NVIC->ISER[1] = 1 << ((PORT4_IRQn) & 31);

    while(1){// Run continously
        if(prepare == currentState){
            // Print wave data to screen, tell user it is on now
            print_Wave_Data(currentWave);
            set_Cursor_LCD(0,1);
            print_String_LCD("Enabled         ");

            // Call function for to load ISR configuration
            set_ISR_Configuration(currentWave);
            

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
        }else if (disable == currentState){
            //Print Dissable message and go to polling function
            set_Cursor_LCD(0,1);
            print_String_LCD("Disabled Press 6");
            currentState = polling;
        }
    }
}

/* set_ISR_Configuration()
 *  Configures Timer ISRs to display desired waves
 *
 * Parameters:
 *  input - WaveData that describes waveform to be used
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
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
    
    // Enable keypad interrupts
    ie_Keypad();
    
    // Set next state
    currentState = output;
}

///////////////////////////////////////////////////////////////////////
//                    Timer Configuration Functions                  //
///////////////////////////////////////////////////////////////////////

/* configure_Timer_XXXXXX()
 *  Sets up timers used for all three waveforms
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - May 3, 2017
 */

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

///////////////////////////////////////////////////////////////////////
//                    Square Wave Interrupt Routine                  //
///////////////////////////////////////////////////////////////////////

/* TA0_0_IRQHandler()
 *  Interrupt for Square wave, toggles value from 5-0V at set interval
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
void TA0_0_IRQHandler(void){
static unsigned int index = 0;                      //Index for arrays
    //Send most recent value to DAC
    send_DAC(ISRData.value[index]);
    //Offset compare value of timer
    TIMER_A0->CCR[0]  +=  ISRData.delay[index];

    // Toggle index
    index ^= 1;

    //Clear flag on exit
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

///////////////////////////////////////////////////////////////////////
//                      Saw Wave Interrupt Routine                   //
///////////////////////////////////////////////////////////////////////

/* TA1_0_IRQHandler()
 *  Interrupt for Saw wave, linear increments value on DAC resets at max
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
void TA1_0_IRQHandler(void){
    static unsigned int currentValue = MIN_VAL_DAC; // Value for DAC

    //Send most recent value to DAC
    send_DAC(currentValue);

    //Increment value for next ISR
    currentValue +=SAW_RESOLUTION;

    //If greater than max value then rest back to minumum
    if(currentValue > MAX_VAL_DAC)
        currentValue = MIN_VAL_DAC;

    //Increment offset
    TIMER_A1->CCR[0] += ISRData.delay[0];

    //Clear flag on exit
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

///////////////////////////////////////////////////////////////////////
//                      Sine Wave Interrupt Routine                  //
///////////////////////////////////////////////////////////////////////

/* TA2_0_IRQHandler()
 *  Interrupt for Sin wave, sets DAC value based on lookup table
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
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


///////////////////////////////////////////////////////////////////////
//                      Keypad Interrupt Routine                     //
///////////////////////////////////////////////////////////////////////

/* PORT4_IRQHandler()
 *  Interrupt for Keypad to stop wave output and begin polling
 *
 * Parameters:
 *  None
 *
 * Errors:
 *  None Currently - May 3, 2017
 */
 
void PORT4_IRQHandler(void){
    //Stop all interrupts
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    TIMER_A2->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;
    id_Keypad();

    //Set state machine to disabled state
    currentState = disable;

    //Clear flag before exit
    COLUMN_PORT->IFG &= ~COLUMN_PINS;
}
