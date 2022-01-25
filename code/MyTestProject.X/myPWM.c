
#include "myPWM.h"
#include <xc.h>

#define MYPWM_MAX 52222

void setupPWM(float percentage1, float percentage2, float percentage3, int direction)
{
        //direction = 0 -> backwards / direction = 1 -> forwards
    
        /* PWM1H1 *, configured to 1kHz, based on fcyc = 26.666 MIPS, Tcycle=37.5nsec/
         * 1ms/37.5nsec = 26666.666 ==> 26666 (fits in 15 bits)
         * of course, we could use a pre-scaler and end up somewhere else
         */
    
    P1TCONbits.PTEN = 0; // Switch off PWM generator
    P1TCONbits.PTCKPS = 0b00; // Sets prescaler, available are 1(00),4(01),16(10) or 64(11)
    P1TPER = MYPWM_MAX/2; //15 bit register
    PWM1CON1bits.PMOD1 = 1; // set PWM unit 1 to independent mode

    if(direction==1){ // backwards
        LATBbits.LATB14 = 0; // Pin 25 = In1 on H bridge
        LATBbits.LATB13 = 1; // Pin 24 = In2
    } else{        // forwards
        LATBbits.LATB14 = 1; // Pin 25 = In1 on H bridge
        LATBbits.LATB13 = 0; // Pin 24 = In2
    }
    
    
    
    PWM1CON1bits.PEN1H = 0; // enable  PWM driver PWM1H1
    PWM1CON1bits.PEN2H = 0; // disable PWM driver
    PWM1CON1bits.PEN3H = 0; // disable PWM driver
    PWM1CON1bits.PEN1L = 1; // disable PWM driver pin number 26
    PWM1CON1bits.PEN2L = 0; // disable PWM driver
    PWM1CON1bits.PEN3L = 0; // disable PWM driver

    P1TCONbits.PTEN = 1; // Switch on PWM generator
    P1DC1 = percentage1*MYPWM_MAX; //PWM1H1 = Pin 25  PWM1L1 = Pin 26 //to get 100% DC, you need to write twice the PER Value (2*26666) -> this is the duty cycle register
    P1DC2 = percentage2*MYPWM_MAX; //PWM1H2 = Pin 23  PWM1L2 = Pin 24
    P1DC3 = percentage3*MYPWM_MAX; //PWM1H3 = Pin 21  PWM1L3 = Pin 22
}