/* 
 * File:   IOconfig.h
 * Author: adamp
 *
 * Created on 13 December 2019, 09:38
 */

#ifndef IOCONFIG_H
#define	IOCONFIG_H


#define LED4 LATBbits.LATB15
#define LED5 LATBbits.LATB14
#define LED6 LATBbits.LATB13
#define LED7 LATBbits.LATB12

#define SW1 !PORTBbits.RB5 


#define LEDON 0
#define LEDOFF 1

void setupIO();

void setupIO_Motor();

#endif	/* IOCONFIG_H */

