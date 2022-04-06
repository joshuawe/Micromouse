/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef ENCODER_H
#define	ENCODER_H

#include <xc.h> // include processor files - each processor file is guarded. 

#ifndef PI 
#define PI 3.14159265359
#endif

/*
 * Impulse pro Umdrehung:   16
 * Übersetzung Getriebe:    33:1
 * Edges pro Puls:          4
 * =======================
 * Edges pro Rotation:      16 * 33 * 4 = 2112
 */
#define PULSES_PER_ROTATION 2112      // Number of encoder pulses for one rotation (360°)
#define WHEEL_RADIUS 30    // [mm]    // 60 mm / 2 :)
#define WHEEL_CIRCUMFERENCE (2 * PI * WHEEL_RADIUS)  // [MM]

// Encoder measurements
extern double speedAngularLeft;         // [rounds/s] wheel turns per second
extern double speedAngularRight;        // [rounds/s] wheel turns per second
extern double WheelDistanceLeft;        // [mm] The distance the wheel covered over ground
extern double WheelDistanceRight;       // [mm] The distance the wheel covered over ground



extern long encoderCountsLeft;          // [-]
extern long encoderCountsRight;         // [-]
extern double speedAbs;                 // [??] the absolute speed: (speedLeft + speedRight)/2
extern double speedLeft;                // [??]    -> mm/s
extern double speedRight;               // [??]    -> mm/s
extern double speedAngularLeft;              // [rounds/s]
extern double speedAngularRight;             // [rounds/s]
extern double WheelDistanceLeft;        // [mm] The distance the wheel covered over ground
extern double WheelDistanceRight;       // [mm]
extern double WheelDistanceLeftAbsolute;     // [mm]  This includes forward and backward motion
extern double WheelDistanceRightAbsolute;    // [mm]
extern double WheelRotationsLeft;            // [-] The num rotations the wheel has turned
extern double WheelRotationsRight;           // [-]
extern double WheelRotationsLeftAbsolute;    // [-] Includes forward and backward rotation
extern double WheelRotationsRightAbsolute;   // [-]


void initQEI(void);
void getEncoderCounts(long* left, long* right);
void updateEncoderCounts(void);
void updateSpeed(void);
void updateWheelDistanceRotation(void);

#endif	/* XC_HEADER_TEMPLATE_H */

