/*
 * File:   encoder.c
 * Author: marius
 *
 * Created on January 25, 2022, 2:49 PM
 */


#include "xc.h"
#include "encoder.h"
//#include <stdlib.h>

extern int deltaT;

int resets1 = 0;
int resets2 = 0;
long encoderCountsLeft = 0;
long encoderCountsRight = 0;
double speedAbs   = 0;   // the absolute speed: sqrt(speedLeft**2 + speedRight**2)
double speedLeft  = 0;
double speedRight = 0;
double WheelDistanceLeft = 0;   // [mm]
double WheelDistanceRight = 0;

void initQEI(void)
{
    // configure analogue pins as digital channels
    //ADPCFGbits.PCFG5 = 1; // QEB on pin 7 shared with AN5
    //ADPCFGbits.PCFG4 = 1; // QEA on pin 6 shared with AN4
    // note: IND pin is not connected on firecracker board
    // Configure QEI module
    QEI1CONbits.QEISIDL = 1; // Discontinue operation when in idle mode
    QEI1CONbits.QEIM = 0b111; // x4 edge gain and reset POSCNT when == MAXCNT
    QEI1CONbits.SWPAB = 1; // Swap the polarity of phase comparison, that forward direction causes positive encoder positions.
    QEI1CONbits.PCDOUT = 0; // Disable counter direction pin (normal I/O operation)
    QEI1CONbits.POSRES = 0; // index pulse does not reset POSCNT
    QEI1CONbits.TQCS = 0; // internal clock source (Tcy)
    DFLT1CONbits.QEOUT = 0; // disable digital filters
    // set initial counter value and maximum range
    MAX1CNT = 0xffff; // set the highest possible time out
    POS1CNT = 0; // set POSCNT into the beginning of range

    // Configure Interrupt controller
    IFS3bits.QEI1IF = 0; // clear interrupt flag
    IEC3bits.QEI1IE = 1; // enable QEI interrupt
    IPC14bits.QEI1IP = 5; // set QEI interrupt priority
    
    // Motor 2 encoder
    QEI2CONbits.QEISIDL = 1; // Discontinue operation when in idle mode
    QEI2CONbits.QEIM = 0b111; // x4 edge gain and reset POSCNT when == MAXCNT
    QEI2CONbits.SWPAB = 0; // Do not swap the polarity of phase comparison
    QEI2CONbits.PCDOUT = 0; // Disable counter direction pin (normal I/O operation)
    QEI2CONbits.POSRES = 0; // index pulse does not reset POSCNT
    QEI2CONbits.TQCS = 0; // internal clock source (Tcy)
    DFLT2CONbits.QEOUT = 0; // disable digital filters
    // set initial counter value and maximum range
    MAX2CNT = 0xffff; // set the highest possible time out
    POS2CNT = 0; // set POSCNT into the beginning of range

    // Configure Interrupt controller
    IFS4bits.QEI2IF = 0; // clear interrupt flag
    IEC4bits.QEI2IE = 1; // enable QEI interrupt
    IPC18bits.QEI2IP = 5; // set QEI interrupt priority
}

// interrupt service routine
void __attribute__((interrupt, auto_psv)) _QEI1Interrupt(void)
{
    // Encoder 1 interrupt
    IFS3bits.QEI1IF = 0;

    if(POS1CNT<MAX2CNT/2)
        resets1++; // over-run condition caused interrupt
    else
        resets1--; // under-run condition caused interrupt
}
   
void __attribute__((interrupt, auto_psv)) _QEI2Interrupt(void) 
{
    // Encoder 2 interrupt
    IFS4bits.QEI2IF = 0;

    if(POS2CNT<MAX2CNT/2)
        resets2++; // over-run condition caused interrupt
    else
        resets2--; // under-run condition caused interrupt

    // TODO: Do we need to reset in the middle again?
    //POS2CNT += 0x7fff;
    
}

void getEncoderCounts(long* left_out, long* right_out) {
    long dist = ((long) resets1) << 16;
    dist += POS1CNT;
    *left_out = dist;
    
    dist = ((long) resets2) << 16;
    dist += POS2CNT;
    *right_out = dist;
}


/*
 * Should be called periodically. Calculates the Encoder counts.
 */
void getEncoderCountsNew(void) {
    long dist = ((long) resets1) << 16;
    dist += POS1CNT;
    encoderCountsLeft = dist;
    
    dist = ((long) resets2) << 16;
    dist += POS2CNT;
    encoderCountsRight = dist;
}


/*
 * This function should be called periodically, as it uses the old distances and new distances and calculates the speed.
 */
void updateSpeed(void) {
    static double WheelDistanceLeftOld;
    static double WheelDistanceRightOld;
    // calculate the new speeds
    speedLeft = (WheelDistanceLeft - WheelDistanceLeftOld) / deltaT;
    speedRight = (WheelDistanceRight - WheelDistanceRightOld) / deltaT;
    // save the current wheel distance for the next speed calculations
    WheelDistanceLeftOld = WheelDistanceLeft;
    WheelDistanceRightOld = WheelDistanceRight;
}