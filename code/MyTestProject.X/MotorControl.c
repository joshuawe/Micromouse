/*
 * File:   MotorControl.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:08
 */

#include "xc.h"
#include "MotorControl.h"
#include "proxSensors.h"
#include "encoder.h"
#include "myPWM.h"

// Necessary variables
extern int delta_t_timer;
double tolerance = 2.0;                   // TO CHANGE!!! [mm] tolerated error between goal and current distance
double influenceProximity = 0.3;          // TO CHANGE!!! [rounds/(mm*s)] estimated influence of proximity measurements on control
double desiredTurningSpeed = 0.25;        // TO CHANGE!!! [rounds/s]
double desiredDrivingSpeed = 1.5;         // TO CHANGE!!! [rounds/s]
double maximumOutput = 1.0;               // TO CHANGE!!! [rounds/s]

// Controller
static PID_Controller pid_velocity_left;        // Controller for velocity of left wheel
static PID_Controller pid_velocity_right;       // Controller for velocity of right wheel
static PID_Controller pid_distance_wall_left;   // Controller for distance to left wall
static PID_Controller pid_distance_wall_right;  // Controller for distance to right wall

// Control cycle with goals of movement
static Control_Cycle cc;

// Encoder measurements
extern double speedAngularLeft;         // [rounds/s] wheel turns per second
extern double speedAngularRight;        // [rounds/s] wheel turns per second
extern double WheelDistanceLeft;        // [mm] The distance the wheel covered over ground
extern double WheelDistanceRight;       // [mm] The distance the wheel covered over ground

// Proximity measurements
static float* distanceRight;
static float* distanceLeft;
static float* distanceFront;
static double measurementDistanceRight = 0;     // [mm] distance from right sensor to wall
static double measurementDistanceLeft = 0;      // [mm] distance from left sensor to wall
static double measurementDistanceFront = 0;     // [mm] distance from front sensor to wall


/*
 Entry point 0 for highlevel code
 * Initialization of controller -> called 1 time before program starts  
 */
void initController()
{
    PID_Controller init;
    init.kFF = 0;                           // TO CHANGE!!! feed-forward factor allows smaller kP
    init.kP = 1;                            // TO CHANGE!!! hint: Ziegler-Nichols method --> kP = 0.45 kU
    init.kI = 1.2*init.kP/delta_t_timer;    // TO CHANGE!!! hint: Ziegler-Nichols method --> kP = 1.2 kP/Pu
    init.kD = 0;                            // TO CHANGE!!! hint: http://robotsforroboticists.com/pid-control/
    init.integralLimit = 1000;              // TO CHANGE!!! limits the integral because the integral slows it down
    init.error = 0;
    init.derivative = 0;
    init.integral = 0;
    init.integralMemory = 0;
    init.errorMemory = 0;
    init.output = 0;
    pid_velocity_left = init;
    pid_velocity_right = init;
    pid_distance_wall_left = init;
    pid_distance_wall_right = init;
    setControllerParameter(pid_velocity_left, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);       // TO CHANGE!!!
    setControllerParameter(pid_velocity_right, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);      // TO CHANGE!!!
    setControllerParameter(pid_distance_wall_left, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);  // TO CHANGE!!!
    setControllerParameter(pid_distance_wall_right, init.kFF, init.kP, init.kI, init.kD, init.integralLimit); // TO CHANGE!!!
}

/*
 Possibility to tune parameters of the four controller
 * either by editing the values in the call of setControllerParameter in initController
 * or by calling the function (also possible from outside ?!) 
 */
void setControllerParameter(PID_Controller pid, double kFF, double kP, double kI, double kD, double integralLimit){
    pid.kFF = kFF;
    pid.kP = kP;
    pid.kI = kI;
    pid.kD = kD;
    pid.integralLimit = integralLimit;
}

/*
 Main control step that receives an error and the corresponding pid controller as input and computes the output
 * called by controlBaseVelocity and controlStraightVelocityBasedOnDistanceMeasurements
 */
void controlStep(PID_Controller pid, double error){
    pid.error = error;
    pid.integral = pid.integralMemory + pid.error * delta_t_timer;
    pid.integral = (pid.integral < pid.integralLimit) ? pid.integral : pid.integralLimit;
    pid.derivative = (pid.error - pid.errorMemory) / delta_t_timer;
    pid.errorMemory = pid.error;
    pid.integralMemory = pid.integral;
    pid.output = pid.kFF * pid.error + pid.kP * pid.error + pid.kI * pid.integral + pid.kD * pid.derivative;
}


/*
 Control of base velocity of the two wheels
 * called by executeControl for turning and straight movement
 */
void controlBaseVelocity()
{    
    double errorSpeedLeft = cc.desiredSpeedLeft - speedAngularLeft;
    double errorSpeedRight = cc.desiredSpeedRight - speedAngularRight;    
    controlStep(pid_velocity_left,errorSpeedLeft);
    controlStep(pid_velocity_right,errorSpeedRight);
}

/*
 Control of velocity based on distance measurement
 * called by executeControl for straight movement only
 */
void controlStraightVelocityBasedOnDistanceMeasurements()
{
    double errorLeft;
    double errorRight;
    
    // no walls to both sides -> just continue with base velocity
    if(measurementDistanceLeft > MAX_POSS_DISTANCE_SENSOR_WALL && measurementDistanceRight > MAX_POSS_DISTANCE_SENSOR_WALL){
        return;
    }
    // no wall on left side
    else if(measurementDistanceLeft > MAX_POSS_DISTANCE_SENSOR_WALL){
        errorLeft = CELL_SIZE - measurementDistanceRight - A_SENSORS - DISTANCE_SENSOR_WALL;
        errorRight = DISTANCE_SENSOR_WALL - measurementDistanceRight;
    }
    // no wall on right side
    else if(measurementDistanceRight > MAX_POSS_DISTANCE_SENSOR_WALL){
        errorLeft = DISTANCE_SENSOR_WALL - measurementDistanceLeft;
        errorRight = CELL_SIZE - measurementDistanceLeft - A_SENSORS - DISTANCE_SENSOR_WALL;
    }
    else{
        errorLeft = DISTANCE_SENSOR_WALL - measurementDistanceLeft;
        errorRight = DISTANCE_SENSOR_WALL - measurementDistanceRight;
    }
    
    controlStep(pid_distance_wall_left,errorLeft);
    controlStep(pid_distance_wall_right,errorRight);
    
}

/*
 Entry point 2 for highlevel code
 * executes the actual control of the current control cycle
 * checks if goal position is already reached
 * controls base velocity depending on the task
 * if no turning movement: controls velocity based on distance measurements to drive straight in the middle of the cell
 * returns 1 if goal position is already reached
 * returns 0 if not
 */
int executeControl()
{
    // check if goal is already reached
    double distanceToGoalLeft = cc.absoluteGoalDistanceLeft - WheelDistanceLeft;
    double distanceToGoalRight = cc.absoluteGoalDistanceRight - WheelDistanceRight;  
    if (distanceToGoalLeft < tolerance && distanceToGoalRight < tolerance){
        return 1;
    }
    
    // control of base velocity
    controlBaseVelocity();
    double outputLeft;
    double outputRight;
    
    // use velocity and proximity control if the mouse is not turning
    if (cc.turn == 0){
        getMeasurements();
        controlStraightVelocityBasedOnDistanceMeasurements();
        outputLeft = pid_velocity_left.output + influenceProximity * pid_distance_wall_left.output;
        outputRight = pid_velocity_right.output + influenceProximity * pid_distance_wall_right.output;
        outputLeft = (outputLeft < maximumOutput) ? outputLeft : maximumOutput;
        outputRight= (outputRight < maximumOutput) ? outputRight : maximumOutput;
        adjustVelocity(outputLeft, outputRight);   
    }
    else{ // use only proximity control if the mouse is turning
        outputLeft = (pid_velocity_left.output < maximumOutput) ? pid_velocity_left.output : maximumOutput;
        outputRight = (pid_velocity_right.output < maximumOutput) ? pid_velocity_right.output : maximumOutput;
        adjustVelocity(outputLeft, outputRight);        
    }
    return 0;
}

/*
 Input measurements of distance sensors
 * write pointer value in local variable
 */
void getMeasurements(){
    getDistances(distanceRight, distanceFront, distanceLeft);
    measurementDistanceRight = *distanceRight;
    measurementDistanceLeft = *distanceLeft;
    measurementDistanceFront = *distanceFront;
}

/*
 Command to motor (PWM Signal)
 */
void adjustVelocity(double outputLeft, double outputRight)
{
    float pwmLeft = convertOutputToPWMsignal(outputLeft);
    float pwmRight = convertOutputToPWMsignal(outputRight);
    setMotorSpeed(pwmLeft, pwmRight);
}

/*
 Helper function to compute PWM Signal
 */
float convertOutputToPWMsignal(double output)
{
    double outputVoltage = output / SPEED_CONSTANT;
    float pwmSignal = outputVoltage / MAX_VOLTAGE;
    return pwmSignal;
}       



/*
 Entry point 1 for highlevel code
 * initializes controller for this control cycle depending on called function:
 * drive_forward_X_cells -> input: double numberOfCells
 * drive_backward_X_cells -> input: double numberOfCells
 * left_X_rad -> input: angleInRad
 * right_X_rad -> input: angleInRad
 * drive_forward
 * drive_backward
 * left_90degree
 * right_90degree
 * turn_around 
 */
void drive_forward_X_cells(double numberOfCells)
{
    initNewControlCycle(1,numberOfCells*CELL_SIZE);
}

void drive_backward_X_cells(double numberOfCells)
{
    initNewControlCycle(2,numberOfCells*CELL_SIZE);
}

void left_X_rad(double angleInRad)
{
    initNewControlCycle(3,angleInRad);
}

void right_X_rad(double angleInRad)
{
    initNewControlCycle(4,angleInRad);
}

void drive_forward()
{
    initNewControlCycle(1,CELL_SIZE);
}

void drive_backward()
{
    initNewControlCycle(2,CELL_SIZE);
}


void left_90degree()
{
    initNewControlCycle(3,PI/4);
}

void right_90degree()
{
    initNewControlCycle(4,PI/4);
}

void turning()
{
    initNewControlCycle(3,PI/2);
}

/*
 Initialization of new control cycle
 * called by functions that are accessed by the highlevel code
 */
void initNewControlCycle(int controlCase, double goalValue)
{
    double relativeGoalDistanceLeft = 0;
    double relativeGoalDistanceRight = 0;
    switch (controlCase){
        case 1:             // drive forward -> goalValue in mm
            relativeGoalDistanceLeft = goalValue;
            relativeGoalDistanceRight = goalValue;
            cc.turn = 0;
            cc.desiredSpeedLeft = desiredDrivingSpeed;
            cc.desiredSpeedRight = desiredDrivingSpeed;
            break;
        case 2:             // drive backward -> goalValue in mm
            relativeGoalDistanceLeft = - goalValue;
            relativeGoalDistanceRight = - goalValue;
            cc.turn = 0;
            cc.desiredSpeedLeft = - desiredDrivingSpeed;
            cc.desiredSpeedRight = - desiredDrivingSpeed;
            break;
        case 3:             // turn x degrees left -> goalValue in rad
            relativeGoalDistanceLeft = - WHEEL_CIRCUMFERENCE * goalValue / (2*PI);
            relativeGoalDistanceRight = WHEEL_CIRCUMFERENCE * goalValue / (2*PI);
            cc.turn = 1;
            cc.desiredSpeedLeft = - desiredTurningSpeed;
            cc.desiredSpeedRight = desiredTurningSpeed;
            break;
        case 4:             // turn x degrees right -> goalValue in rad
            relativeGoalDistanceLeft = WHEEL_CIRCUMFERENCE * goalValue / (2*PI);
            relativeGoalDistanceRight = - WHEEL_CIRCUMFERENCE * goalValue / (2*PI);
            cc.turn = 1;
            cc.desiredSpeedLeft = desiredTurningSpeed;
            cc.desiredSpeedRight = - desiredTurningSpeed;
            break;
    }
    cc.absoluteGoalDistanceLeft = WheelDistanceLeft + relativeGoalDistanceLeft;
    cc.absoluteGoalDistanceRight = WheelDistanceRight + relativeGoalDistanceRight;
}
