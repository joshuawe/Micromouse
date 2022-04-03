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
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  


#define CELL_SIZE 180               // length and width of one cell in mm
#define A_WHEELS 102                // distance between wheels in mm -> CHANGE THIS!!! (from CAD)
#define A_SENSORS 64                // distance between sensors in mm -> CHANGE THIS!!! (from CAD)
#define DISTANCE_SENSOR_WALL ((CELL_SIZE - A_SENSORS) / 2)          // distance sensor to wall in mm -> CHANGE THIS!!! (from CAD)
#define MAX_POSS_DISTANCE_SENSOR_WALL (1.5 * (CELL_SIZE - A_WHEELS/2 - A_SENSORS/2))     // CHANGE THIS!!! (factor 1.5 is random value > 1
                                                                                         // that ensures that there really is no wall)
#define A_SIDE_SENSORS_AXIS 50.4    // CHANGE THIS!!! (from CAD) distance between motor axis and sensors to the side
                                    // -> used for calibration, so it is important that this distance is correct!
#define CALIBRATION_SIDE (CELL_SIZE/2 + A_SIDE_SENSORS_AXIS)        // remaining distance to goal position
                                                                    // alternatively CHANGE THIS if it is more accurate to measure the
                                                                    // current position of the mouse if the missing wall is detected
#define A_FRONT_SENSOR_AXIS 69.9    // CHANGE THIS!!! (from CAD) distance between motor axis and sensor to the front
                                    // -> used for calibration, so it is important that this distance is correct!
#define DISTANCE_USED_TO_CALIBRATE 50       // distance front sensor to wall when position is new calibrated
#define CALIBRATION_FRONT (A_FRONT_SENSOR_AXIS + DISTANCE_USED_TO_CALIBRATE - CELL_SIZE/2)       // remaining distance to goal position




/*
 * Speed Constant kn from Datasheet:    1130 [Motor rpm/V]
 * Übersetzung Getriebe:                33
 * Umrechnung in pro Minute:            60
 * =======================
 * Speed Constant umgerechnet:          1130 / 60 / 33 = 0.5707 [Wheel rounds per second / V]
 */
#define SPEED_CONSTANT 0.5707       // [wheel rps/V]
#define MAX_VOLTAGE 9.0             // maximum voltage -> leads to a maximum speed of 3.99 round/s


typedef struct{
    double kFF;
    double kP;
    double kI;
    double kD;
    double integralLimit;
    double error;
    double integral;
    double derivative;
    double errorMemory;
    double integralMemory;
    double output;
} PID_Controller;

typedef struct{
    double desiredSpeedLeft;
    double desiredSpeedRight;
    double absoluteGoalDistanceLeft;
    double absoluteGoalDistanceRight;
    int turn;
} Control_Cycle;

typedef struct{
    Control_Cycle controlCycle;
    double velocity_left_error;
    double velocity_left_output;
    double velocity_right_error;
    double velocity_right_output;
    double distance_wall_left_error;
    double distance_wall_left_output;
    double distance_wall_right_error;
    double distance_wall_right_output;
    double distance_to_goal_left;
    double distance_to_goal_right;
    double distance_to_goal;
} Outputs;

/*
 PART 0 
 */
void initController();
void setControllerParameter(PID_Controller pid, double kFF, double kP, double kI, double kD, double integralLimit);
void controlStep(PID_Controller pid, double error);

/*
 PART 1 
 */
//void drive_forward_X_cells(double numberOfCells);
//void drive_backward_X_cells(double numberOfCells);
void left_X_rad(double angleInRad);
void right_X_rad(double angleInRad);
void drive_forward();
void drive_backward();
void left_90degree();
void right_90degree();
void turning();
void initNewControlCycle(int controlCase, double goalValue);


/*
 PART 2 
 */
int executeControl();
int checkGoalReachedAlready();
void getMeasurements();
void calibrateAndControlStraightVelocityBasedOnDistanceMeasurements();
void calibrateGoalSide();
void calibrateGoalFront();
void controlBaseVelocity();
double checkOutputIfItExceedsMaximum(double output);
void adjustVelocity(double outputLeft, double outputRight);
float convertOutputToPWMsignal(double output);


#endif	/* XC_HEADER_TEMPLATE_H */

