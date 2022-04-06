#include <stdio.h>
#include "myTimers.h"
#include "IOconfig.h"
#include "encoder.h"
#include "dma.h"
#include "myPWM.h"
#include "proxSensors.h"
#include "serialComms.h"
#include "MotorControl.h"

static int myCount;
int t;

int const delta_t_timer = 100;   // time in [ms] determining the frequency, sorry for the ugly name
double const delta_t_sec = 0.1;     // [s]

void initTimer1(unsigned int period)
{

    myCount = 0;
    t = period;
    T1CON = 0;              // ensure Timer 1 is in reset state

    // *** prescaler TCKPS: 11 --> 1:256, 10 --> 1:64, 01 --> 1:8, 00 --> 1:1
    //unsigned int scale = period >> 11;

    if(period<3){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6 * 8 * 8;
        T1CONbits.TCKPS = 0b00;
    }
    else if(period<20){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6 * 8;
        T1CONbits.TCKPS = 0b01;
    }
    else if(period<158){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6;
        T1CONbits.TCKPS = 0b10;
    }
    else if(period<630){ // max Number = 16 777 216 = 2^24 --> period < 630
        period = period * 416.6 / 4;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*2){ // divide period, change LED only every second time
        period = period * 416.6 / 4 / 2;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*4){
        period = period * 416.6 / 4 / 4;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*8){
        period = period * 416.6 / 4 / 8;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*16){
        period = period * 416.6 / 4 / 16;
        T1CONbits.TCKPS = 0b11;
    }




    /*if (period <= 39)
    {
        T1CONbits.TCKPS = 0b00; // FCY divide by 1: tick = 37.5ns
        period = period * 0.0000375;
    }
    else if (period > 39 && period <= 312)
    {
        T1CONbits.TCKPS = 0b01; // FCY divide by 8: tick = 0.3us
        period = period * 0.0003;

    }
    else if (period > 312 && period <= 2500)
    {
        T1CONbits.TCKPS = 0b10; // FCY divide by 64: tick = 2.4us
        period = period * 0.0024;
    }
    else if (period > 2500)
    {
        T1CONbits.TCKPS = 0b11; // FCY divide by 256: tick = 9.6us
        period = period * 0.0096;
    }*/

    //period = period * 416.6;

    //T1CONbits.TCKPS = 0b10; // FCY divide by 64: tick = 2.4us (Tcycle=37.5ns)
    T1CONbits.TCS = 0;      // select internal FCY clock source
    T1CONbits.TGATE = 0;    // gated time accumulation disabled
    TMR1 = 0;
    PR1 = period;           // set Timer 1 period register ()
    IFS0bits.T1IF = 0;      // reset Timer 1 interrupt flag
    IPC0bits.T1IP = 4;      // set Timer1 interrupt priority level to 4
    IEC0bits.T1IE = 1;      // enable Timer 1 interrupt
    T1CONbits.TON = 0;      // leave timer disabled initially
}



void startTimer1(void)
{
    T1CONbits.TON = 1;
}

void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    // Reset timer 1 interrupt flag
    IFS0bits.T1IF = 0;
    
    updateDistances();
    updateEncoderCounts();
    updateWheelDistanceRotation();
    updateSpeed();
    
    executeControl();
    
    //setMotorSpeed(0,0);
 
    // for SerialStudio
    printf("/*%f,%f,%f,%ld,%ld,%f,%f,%f,%f,%f,%f,%f*/\r\n", distanceLeft, distanceFront, distanceRight, 
        encoderCountsLeft, encoderCountsRight, 
        WheelDistanceLeft, WheelDistanceRight, 
        speedAngularLeft, speedAngularRight,
        speedLeft, speedRight, distanceToGoalLeft, distanceToGoalRight);

    

    //LED1 = ~LED1;
    if (BUTTON) {
        //LED2 = ~LED2;
        //initNewControlCycle(2,100);
        right_90degree();
    }
}
