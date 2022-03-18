/* 
 * File:   IOconfig.h
 * Author: adamp
 *
 * Created on 13 December 2019, 09:38
 */

#ifndef IOCONFIG_H
#define	IOCONFIG_H


// LED1 is pin 33
// LED2 is pin 32
#define LED1 LATBbits.LATB4
#define LED2 LATAbits.LATA8

#define SW1 !PORTBbits.RB5 

#define LEDON 1
#define LEDOFF 2

void setupIO();

void setupIO_Motor();

#endif	/* IOCONFIG_H */

