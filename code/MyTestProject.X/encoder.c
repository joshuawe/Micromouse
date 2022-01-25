/*
 * File:   encoder.c
 * Author: marius
 *
 * Created on January 25, 2022, 2:49 PM
 */


#include "xc.h"
#include "encoder.h"

long longPOS = 0;

void init_QEI(void)
{
    // configure analogue pins as digital channels
    ADPCFGbits.PCFG5 = 1; // QEB on pin 7 shared with AN5
    ADPCFGbits.PCFG4 = 1; // QEA on pin 6 shared with AN4
    // note: IND pin is not connected on firecracker board
    // Configure QEI module
    QEICONbits.QEISIDL = 1; // Discontinue operation when in idle mode
    QEICONbits.QEIM = 0b111; // x4 edge gain and reset POSCNT when == MAXCNT
    QEICONbits.SWPAB = 0; // Do not swap the polarity of phase comparison
    QEICONbits.PCDOUT = 0; // Disable counter direction pin (normal I/O operation)
    QEICONbits.POSRES = 0; // index pulse does not reset POSCNT
    QEICONbits.TQCS = 0; // internal clock source (Tcy)
    DFLTCONbits.QEOUT = 0; // disable digital filters
    // set initial counter value and maximum range
    MAXCNT = 0xffff; // set the highest possible time out
    POSCNT = 0x7fff; // set POSCNT into middle of range

    // Configure Interrupt controller
    IFS3bits.QEI1IF = 0; // clear interrupt flag
    IEC3bits.QEI1IE = 1; // enable QEI interrupt
    IPC14bits.QEI1IP = 5; // set QEI interrupt priority
}

// interrupt service routine
void __attribute__((interrupt, auto_psv)) _QEIInterrupt(void)
{
    IFS3bits.QEI1IF = 0; // clear interrupt flag
    if(POSCNT<32768)
    longPOS++; // over-run condition caused interrupt
    else
    longPOS--; // under-run condition caused interrupt
}