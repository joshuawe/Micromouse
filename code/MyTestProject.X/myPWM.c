
#include "myPWM.h"
#include <xc.h>

#define MYPWM_MAX 52222

void setupPWM() {
    // Setup PWM for left motor
    P1TCONbits.PTEN = 0; // Switch off PWM generator
    P1TCONbits.PTCKPS = 0b00; // Sets prescaler, available are 1(00),4(01),16(10) or 64(11)
    P1TPER = MYPWM_MAX/2; //15 bit register
    PWM1CON1bits.PMOD1 = 1; // set PWM unit 1 to independent mode

    PWM1CON1bits.PEN1H = 1; // enable  PWM driver PWM1H1 (right motor)
    PWM1CON1bits.PEN2H = 1; // enable PWM driver PWM1H2 (left motor)
    PWM1CON1bits.PEN3H = 0; // disable PWM driver
    PWM1CON1bits.PEN1L = 0; // disable PWM driver
    PWM1CON1bits.PEN2L = 0; // disable PWM driver
    PWM1CON1bits.PEN3L = 0; // disable PWM driver
    
    // Just to be sure, disable PWM2 module
    P2TCONbits.PTEN = 0;
    PWM2CON1bits.PEN1H = 0; // disable PWM driver
    PWM2CON1bits.PEN1L = 0; // disable PWM driver
    
    // Set zero values for initialization
    setPWM(0, DIRECTION_NONE, 0, DIRECTION_NONE);
    
    // Switch on PWM generator
    P1TCONbits.PTEN = 1; 
}

void setPWM(float left_motor, int left_direction, float right_motor, int right_direction)
{
    // Motor 1 configuration
    // TODO: Is this left or right?
    //TRISCbits.TRISC5 = 0;  // Direction 1, pin 38, RC5
    //TRISCbits.TRISC4 = 0;  // Direction 2, pin37, RC4
    //TRISBbits.TRISB12 = 0; // PWM 1, pin 10, PWM1H, RB12
    //TRISBbits.TRISB15 = 0; // output pin 26
    //LATCbits.LATC5 = 0;  // Pin 38
    //LATCbits.LATC4 = 0;  // Pin 37
    //LATBbits.LATB12 = 0; // Pin 10
    //LATBbits.LATB15 = 0; // 
    
    // Left motor direction
    LATCbits.LATC5 = 0; // Pin 38 = In1 on H bridge
    LATCbits.LATC4 = 0; // Pin 37 = In2
    
    if(left_direction==DIRECTION_BWD) {
        LATCbits.LATC4 = 1; // Pin 37 = In2
    } else if(left_direction==DIRECTION_FWD) {
        LATCbits.LATC5 = 1; // Pin 38 = In1 on H bridge
    }
    
    // Right motor direction
    LATBbits.LATB9 = 0;  // Pin 1
    LATCbits.LATC6 = 0;  // Pin 2
    
    if(right_direction==DIRECTION_BWD) {
        LATBbits.LATB9 = 1; // Pin 1 = In2
    } else if(right_direction==DIRECTION_FWD) {
        LATCbits.LATC6 = 1; // Pin 2 = In1 on H bridge
    }
    
    // Motor speed
    //to get 100% DC, you need to write twice the PER Value (2*26666) -> this is the duty cycle register
    P1DC1 = right_motor*MYPWM_MAX;
    P1DC2 = left_motor*MYPWM_MAX;
}
