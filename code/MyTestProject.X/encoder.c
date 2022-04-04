/*
 * File:   encoder.c
 * Author: marius
 *
 * Created on January 25, 2022, 2:49 PM
 */


#include "xc.h"
#include "encoder.h"
#include <stdlib.h>

extern int delta_t_timer;               // [s]

int resets1 = 0;  // [-]
int resets2 = 0;  // [-]
long encoderCountsLeft  = 0;  // [-]
long encoderCountsRight = 0;  // [-]
double speedAbs   = 0;   // [??] the absolute speed: (speedLeft + speedRight)/2
double speedLeft  = 0;   // [??]    -> mm/s
double speedRight = 0;   // [??]    -> mm/s
double speedAngularLeft = 0;              // [rounds/s]
double speedAngularRight = 0;             // [rounds/s]
double WheelDistanceLeft           = 0;   // [mm] The distance the wheel covered over ground
double WheelDistanceRight          = 0;   // [mm]
double WheelDistanceLeftAbsolute   = 0;   // [mm]  This includes forward and backward motion
double WheelDistanceRightAbsolute  = 0;   // [mm]
double WheelRotationsLeft          = 0;  // [-] The num rotations the wheel has turned
double WheelRotationsRight         = 0;  // [-]
double WheelRotationsLeftAbsolute  = 0;  // [-] Includes forward and backward rotation
double WheelRotationsRightAbsolute = 0;  // [-]

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

    if(POS1CNT<MAX1CNT/2)
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
 * Should be called periodically. Calculates the Encoder counts and makes them globally available.
 */
void updateEncoderCounts(void) {
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
    static double WheelRotationsLeftOld;
    static double WheelRotationsRightOld;
    // calculate the new speeds
    speedLeft = (WheelDistanceLeft - WheelDistanceLeftOld) / delta_t_timer;
    speedRight = (WheelDistanceRight - WheelDistanceRightOld) / delta_t_timer;
    speedAbs = (speedLeft + speedRight) / 2;
    speedAngularLeft = (WheelRotationsLeft - WheelRotationsLeftOld) / delta_t_timer;
    speedAngularRight = (WheelRotationsRight - WheelRotationsRightOld) / delta_t_timer;
    // save the current wheel distance for the next speed calculations
    WheelDistanceLeftOld = WheelDistanceLeft;
    WheelDistanceRightOld = WheelDistanceRight;
    WheelRotationsLeftOld = WheelRotationsLeft;
    WheelRotationsRightOld = WheelRotationsRight;
}


/*
 * Should be called periodically. Updates the distance covered by the wheels as well as the number of rotations of the wheel.
 */
void updateWheelDistanceRotation(void) {
    // Defining the 'old' values from previous function calls
    static long encoderCountsLeftOld, encoderCountsRightOld;
    static long WheelRotationsLeftOld, WheelRotationsRightOld;
    long temp = 0;
    double dTemp = 0;
    // First, the Wheel Rotations are calculated
    temp = (encoderCountsLeft - encoderCountsLeftOld) / PULSES_PER_ROTATION;
    WheelRotationsLeft += temp;
    WheelRotationsLeftAbsolute += abs(temp);
    temp = (encoderCountsRight - encoderCountsRightOld) / PULSES_PER_ROTATION;
    WheelRotationsRight += temp;
    WheelRotationsRightAbsolute += abs(temp);
    // Second, the distance travelled by the wheel is calculated
    dTemp = (WheelRotationsLeft - WheelRotationsLeftOld) * WHEEL_CIRCUMFERENCE;
    WheelRotationsLeftOld += dTemp;
    WheelDistanceLeft += dTemp;
    WheelDistanceLeftAbsolute += abs(dTemp);
    dTemp = (WheelRotationsRight - WheelRotationsRightOld) * WHEEL_CIRCUMFERENCE;
    WheelDistanceRight += dTemp;
    WheelDistanceRightAbsolute += abs(dTemp);
    // Last, the current values are stored for the next function call
    encoderCountsLeftOld = encoderCountsLeft;
    encoderCountsRightOld = encoderCountsRight;
    WheelRotationsLeftOld = WheelRotationsLeft;
    WheelRotationsRightOld = WheelRotationsRight;
}