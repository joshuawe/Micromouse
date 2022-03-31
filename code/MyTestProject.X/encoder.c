/*
 * File:   encoder.c
 * Author: marius
 *
 * Created on January 25, 2022, 2:49 PM
 */


#include "xc.h"
#include "encoder.h"
//#include <stdlib.h>

int resets1 = 0;
int resets2 = 0;
//unsigned long totalCountsLeft, totalCountsRight;   // will store the absolute Encoder counts
//unsigned long lastCountsLeft, lastCountsRight;     // will store the Encoder counts from previous function call

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
    POS1CNT = 0x7fff; // set POSCNT into middle of range

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
    POS2CNT = 0x7fff; // set POSCNT into middle of range

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
    long dist = resets1 << 16;
    dist += POS1CNT - 0x7fff;
    *left_out = dist;
    
    dist = resets2 << 16;
    dist += POS2CNT - 0x7fff;
    *right_out = dist;
}

//void addEncoderCounts() {    
//    long left_out, right_out;  // current encoder values
//    // retrieve current encoder count values
//    getEncoderCounts(left_out, right_out);
//    // add the absolute encoder count difference to our counters
//    totalCountsLeft += abs(lastCountsLeft - left_out);
//    totalCountsRight += abs(lastCountsRight - right_out);
//    // Save the current encoder values for the next function call
//    lastCountsLeft = (unsigned long) left_out;
//    lastCountsRight = (unsigned long) right_out;    
//}
