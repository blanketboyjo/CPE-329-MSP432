/*  Assignment 6: Operation Timing
 *    This code tests various operations and data types
 *    to see how different operations and data types vary
 *    in execution time. This is important to understand
 *    how differences in how code is written affect overall
 *    speed.
 *
 *    Dependencies:
 *      msp.h   - Access to pins on board
 *      math.h  - Access to abs, sqrt, sin, and sinh
 *
 *    Errors:
 *      None Currently Apr 26, 2017
 *
 *    Revisions:
 *      Apr 26,  2017 - Initial Creation
 *
 *    Authors:
 *      Jordan Jones
 *      Andrew Hartley 
 */

///////////////////////////////////////////////////////////////////////
//                        Depenendency Includes                      //
/////////////////////////////////////////////////////////////////////// 
#include "msp.h"
#include "math.h"

///////////////////////////////////////////////////////////////////////
//                        Mode Defines (DO NOT EDIT)                 //
///////////////////////////////////////////////////////////////////////
#define SUBROUTINE 
#define N          x = n;
#define N_PLUS_1   x = n + 1;
#define N_PLUS_7   x = n + 7;
#define N_MULT_2   x = n * 2;
#define N_MULT_3   x = n * 3;
#define N_DIV_2    x = n / 2;
#define N_DIV_3    x = n / 3;
#define SIN(N)     x = sin(n);
#define SINH(N)    x = sinh(n);
#define SQRT(N)    x = sqrt(n);
#define ABS(N)     x = abs(n);


///////////////////////////////////////////////////////////////////////
//                        Running mode (EDITABLE)                    //
///////////////////////////////////////////////////////////////////////

//Select datatype for operations
#define VAR_TYPE    double

//Select operation
#define OPERATION        ABS(N)



VAR_TYPE TestFunction(VAR_TYPE n);

///////////////////////////////////////////////////////////////////////
//                        Main function                              //
///////////////////////////////////////////////////////////////////////

int main(void) {
    volatile VAR_TYPE y;                //Volatile ensures variable not optimized away
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
    P1->SEL1 &= ~BIT0;                  //set P1.0 as simple I/O
    P1->SEL0 &= ~BIT0;
    P1->DIR |= BIT0;                    //set P1.0 as output
    P2->SEL1 &= ~(BIT2 | BIT1 | BIT0);  //set P2.0-2.2 as simple I/O
    P2->SEL0 &= ~(BIT2 | BIT1 | BIT0);
    P2->DIR |= (BIT2 | BIT1 | BIT0);    //set P2.0-2.2 as output pins
    P2->OUT |= (BIT2 | BIT1 | BIT0);    // turn on RGB LED
    y = TestFunction(15);               // test function for timing
    P2->OUT &= ~(BIT2 | BIT1 | BIT0);   // turn off RGB LED
}

///////////////////////////////////////////////////////////////////////
//                          Testing Function                         //
///////////////////////////////////////////////////////////////////////
VAR_TYPE TestFunction(VAR_TYPE n) {
    VAR_TYPE x = 0;
    P1->OUT |= BIT0; // set P1.0 LED on

    //Use fill in operation based on preprocessor defines
    OPERATION

    P1->OUT &= ~BIT0; // set P1.0 LED off
    return x;
}
