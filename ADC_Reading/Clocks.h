/*
 * Clocks.h
 *
 * Holds functions for generating changing CPU clock speed and generating delays
 *  set_DCO - change CPU speed
 *  delay_ms - delay milliseconds
 *  delay_ns - delay nanoseconds
 *
 * Dependencies:
 *  MSP.h
 *
 * Errors:
 *  -FIXED May 3, 2017- 48 MHZ is not supported and will lock processor - Apr 5, 2017
 *  Fastest Delay is 1.9 uS - Apr 10, 2017
 *
 * Revisions:
 *  Apr 5, 2017 - Initial Creation
 *  Apr 7, 2017 - Nanoseconds Added
 *  Apr 10,2017 - Nanoseconds repaired
 *  May 3, 2017 - 48MHZ added to set_DCO
 *
 * Authors: Drew Hartley, Jordan Jones
 */
#ifndef CLOCKS_H_
#define CLOCKS_H_
////////////////////////////////////////////////////////////////////////
//                      Non-editable Defines                          //
////////////////////////////////////////////////////////////////////////

//Frequency Defines
#define FREQ_1_5_MHZ 0
#define FREQ_3_MHZ 1
#define FREQ_6_MHZ 2
#define FREQ_12_MHZ 3
#define FREQ_24_MHZ 4
#define FREQ_48_MHZ 5
//Millisecond loop count defines
#define DELAY_MS_1_5_MHZ 134
#define DELAY_MS_3_MHZ 269
#define DELAY_MS_6_MHZ 546
#define DELAY_MS_12_MHZ 1093
#define DELAY_MS_24_MHZ 2191
#define DELAY_MS_48_MHZ 4368
//Nanosecond division variable
#define PERIOD_NS_1_5_MHZ 4100

////////////////////////////////////////////////////////////////////////
//              Clock Speed Control                                   //
////////////////////////////////////////////////////////////////////////

/* set_DCO()
 * Configures CPU clock to set values defined above
 *
 * Parameters:
 * FCPU - FCPU frequency based on defined values
 *
 * Returns:
 * 0 - No Error
 * -1 - Frequency Error
 *
 * Errors:
 * -FIXED May 4, 2017- 48MHz is currently not supported - Apr 5, 2017
 */
 
int set_DCO(uint32_t FCPU){
    uint32_t tempDCO = 0;
    switch(FCPU){//Set temporary variable to correct DCORSEL
    case FREQ_1_5_MHZ:
        tempDCO = CS_CTL0_DCORSEL_0;
    break;
    case FREQ_3_MHZ:
        tempDCO = CS_CTL0_DCORSEL_1;
    break;
    case FREQ_6_MHZ:
        tempDCO = CS_CTL0_DCORSEL_2;
    break;
    case FREQ_12_MHZ:
        tempDCO = CS_CTL0_DCORSEL_3;
    break;
    case FREQ_24_MHZ:
        tempDCO = CS_CTL0_DCORSEL_4;
    break;
    case FREQ_48_MHZ:
        /* Step 1: Transition to VCORE Level 1: AM0_LDO --> AM1_LDO */
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));
            PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_1;
        while ((PCM->CTL1 & PCM_CTL1_PMR_BUSY));

        /* Step 2: Configure Flash wait-state to 1 for both banks 0 & 1 */
        FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
                FLCTL_BANK0_RDCTL_WAIT_1;
        FLCTL->BANK1_RDCTL  = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK1_RDCTL_WAIT_MASK)) |
                FLCTL_BANK1_RDCTL_WAIT_1;

        /* Step 3: Configure DCO to 48MHz, ensure MCLK uses DCO as source*/
        CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
        CS->CTL0 = 0;                           // Reset tuning parameters
        CS->CTL0 = CS_CTL0_DCORSEL_5;           // Set DCO to 48MHz
        /* Select MCLK = DCO, no divider */
        CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK) |
                CS_CTL1_SELM_3;
        CS->KEY = 0;                            // Lock CS module from unintended accesses
        return 0;

    default: //Invalid frequency given
        return -1; //Leave function return -1
    }
    CS->KEY = CS_KEY_VAL;
    CS->CTL0 = 0;
    CS->CTL0 = tempDCO;
    CS->CTL1 = CS_CTL1_SELA_2| CS_CTL1_SELS_3 | CS_CTL1_SELM_3;
    CS->KEY = 0;

    return 0;
}

////////////////////////////////////////////////////////////////////////
//                   Delay Subroutines                                //
////////////////////////////////////////////////////////////////////////

/* delay_ms()
 * Delays for a set amount of milliseconds
 *
 * Parameters:
 * delay - Time in milliseconds
 * FCPU - CPU frequency
 *
 * Returns:
 * 0 - No Error
 * -1 - Frequency Error
 *
 * Errors:
 * None Currently - Apr 5, 2017
 */
 
int delay_ms(uint32_t delay, uint32_t FCPU){
    uint32_t i,ms,delayCount;
    switch(FCPU){//Set inner loop value based on given frequency
    case FREQ_1_5_MHZ:
        delayCount = DELAY_MS_1_5_MHZ;
    break;
    case FREQ_3_MHZ:
        delayCount = DELAY_MS_3_MHZ;
    break;
    case FREQ_6_MHZ:
        delayCount = DELAY_MS_6_MHZ;
    break;
    case FREQ_12_MHZ:
        delayCount = DELAY_MS_12_MHZ;
    break;
    case FREQ_24_MHZ:
        delayCount = DELAY_MS_24_MHZ;
    break;
    case FREQ_48_MHZ:
        delayCount = DELAY_MS_48_MHZ;
    break;
    default:
    return -1; //Return -1 if frequency is not valid
    }
    for(ms = 0; ms < delay; ms++)//Loop for # of ms desired
        for(i = 0; i < delayCount; i++);//Loop for ~1 ms
    return 0;
}

/* delay_ms()
 * Delays for a set amount of milliseconds
 *
 * Parameters:
 * delay - Time in milliseconds
 * FCPU - CPU frequency
 *
 * Returns:
 * Void
 *
 * Errors:
 * None Currently - Apr 10, 2017
 * */
 
void delay_ns(uint32_t delay, uint32_t FCPU){
    uint32_t i, delayCount;
    delayCount = (((delay << FCPU)/PERIOD_NS_1_5_MHZ));
    for(i = 8; i< delayCount; i++);
}

#endif /* CLOCKS_H_ */

