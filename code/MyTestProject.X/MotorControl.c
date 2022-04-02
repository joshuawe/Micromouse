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
#include "math.h"

#define PI 3.14159265359lf

const double cellSize = 180;            // length and width of each cell in maze in mm
const double rWheels = 30;              // radius of each wheel in mm
const double UWheels = 2*PI*rWheels;    // circumference of each wheel in mm
const double aWheels = 102;             // distance between wheels in mm -> CHANGE THIS!!! (from CAD)
const double aSensors = 64;             // distance between sensors in mm -> CHANGE THIS!!! (from CAD)
const double defaultDistanceSensorWall = 58;     // distance sensor to wall in mm -> CHANGE THIS!!! (from CAD)
const double maxPossibleDistanceToWall = 1.5 * (cellSize - aWheels/2 - aSensors/2);     // CHANGE THIS!!! (factor 1.5 is random value > 1 that ensures that there really is no wall)

// Other necessary variables
extern int delta_t_timer;
const double integralLimit = 1000;              // TO CHANGE!!!
const double tolerance = 1;                     // TO CHANGE!!! tolerated error between goal and current distance in mm
const double influenceProximity = 1;            // TO CHANGE!!! influence of proximity measurements on control
const double convertOutputToPWMsignal = 1;      // TO CHANGE!!! output in mm/s should be transformed to % PWM

// Controller
static PID_Controller pid_base_velocity;
static PID_Controller pid_velocity_left;
static PID_Controller pid_velocity_right;
static PID_Controller pid_distance_wall_left;
static PID_Controller pid_distance_wall_right;


// Encoder
extern double speedAbs;                 // [??] the absolute speed: (speedLeft + speedRight)/2
extern double speedLeft;                // [??]
extern double speedRight;               // [??]
extern double WheelDistanceLeft;        //[mm] The distance the wheel covered over ground
extern double WheelDistanceRight;       //[mm] The distance the wheel covered over ground

// Control cycle with goals
static Control_Cycle cc;

// Proximity measurements
static double distanceRight;
static double distanceLeft;
static double distanceFront;


void initController()
{
    PID_Controller init;
    init.kP = 1;                            // hint: Ziegler-Nichols method --> kP = 0.45 kU
    init.kI = 1.2*init.kP/delta_t_timer;    // hint: Ziegler-Nichols method --> kP = 1.2 kP/Pu
    init.kD = 0;                            // hint: http://robotsforroboticists.com/pid-control/
    init.integralLimit = integralLimit;
    init.tolerance = tolerance;
    pid_velocity_left = {init.kP, init.kI, init.kD, init.integralLimit,init.tolerance,0,0,0,0,0,0};
    pid_velocity_right = {init.kP, init.kI, init.kD, init.integralLimit,init.tolerance,0,0,0,0,0,0};
    pid_distance_wall_left = {init.kP, init.kI, init.kD, init.integralLimit,init.tolerance,0,0,0,0,0,0};
    pid_distance_wall_right = {init.kP, init.kI, init.kD, init.integralLimit,init.tolerance,0,0,0,0,0,0};
}

void setControllerParameter(PID_Controller pid, double kP, double kI, double kD, double integralLimit, double tolerance){
    pid.kP = kP;
    pid.kI = kI;
    pid.kD = kD;
    pid.integralLimit = integralLimit;
    pid.tolerance = tolerance;
}


static void controlStep(PID_Controller pid, double error){
    pid.error = error;
    pid.integral = pid.integralMemory + pid.error * delta_t_timer;
    pid.derivative = (pid.error - pid.errorMemory) / delta_t_timer;
    pid.errorMemory = pid.error;
    pid.integralMemory = pid.integral;
    pid.output = pid.kP * pid.error + pid.kI * pid.integral + pid.kD * pid.derivative;
}


static void initNewControlCycle(int controlCase, double goalValue)
{
    double relativeGoalDistanceLeft = 0;
    double relativeGoalDistanceRight = 0;
    switch (controlCase){
        case 1:             // drive forward -> goalValue in mm
            relativeGoalDistanceLeft = goalValue;
            relativeGoalDistanceRight = goalValue;
            cc.turn = 0;
            break;
        case 2:             // drive backward -> goalValue in mm
            relativeGoalDistanceLeft = - goalValue;
            relativeGoalDistanceRight = - goalValue;
            cc.turn = 0;
            break;
        case 3:             // turn x degrees left -> goalValue in rad
            relativeGoalDistanceLeft = - UWheels * goalValue / (2*PI);
            relativeGoalDistanceRight = UWheels * goalValue / (2*PI);
            cc.turn = 1;
            break;
        case 4:             // turn x degrees right -> goalValue in rad
            relativeGoalDistanceLeft = UWheels * goalValue / (2*PI);
            relativeGoalDistanceRight = - UWheels * goalValue / (2*PI);
            cc.turn = 1;
            break;
    }
    cc.absoluteGoalDistanceLeft = WheelDistanceLeft + relativeGoalDistanceLeft;
    cc.absoluteGoalDistanceRight = WheelDistanceRight + relativeGoalDistanceRight;
}


static int controlBaseVelocity()
{    
    double errorLeft = cc.absoluteGoalDistanceLeft - WheelDistanceLeft;
    double errorRight = cc.absoluteGoalDistanceRight - WheelDistanceRight;
    
    // goal already reached
    if (errorLeft < tolerance && errorRight < tolerance){
        return 1;
    }
    controlStep(pid_velocity_left,errorLeft);
    controlStep(pid_velocity_right,errorRight);
    return 0;
}

static void controlStraightVelocityBasedOnDistanceMeasurements()
{
    double errorLeft;
    double errorRight;
    
    // no walls to each side -> just continue with base velocity
    if(distanceLeft > maxPossibleDistanceToWall && distanceRight > maxPossibleDistanceToWall){
        return 0;
    }
    // no wall on left side
    else if(distanceLeft > maxPossibleDistanceToWall){
        errorRight = defaultDistanceSensorWall - distanceRight;
        errorLeft = cellSize - errorRight;
    }
    // no wall on right side
    else if(distanceRight > maxPossibleDistanceToWall){
        errorLeft = defaultDistanceSensorWall - distanceLeft;
        errorRight = cellSize - errorLeft;
    }
    else{
        errorLeft = defaultDistanceSensorWall - distanceLeft;
        errorRight = defaultDistanceSensorWall - distanceRight;
    }
    
    controlStep(pid_distance_wall_left,errorLeft);
    controlStep(pid_distance_wall_right,errorRight);
    
}


int executeControl(){
    int goalReached = controlBaseVelocity();
    // goal is already reached
    if (goalReached == 1){
        return 1;
    }
    
    double outputLeft = 0;
    double outputRight = 0;
    // only use proximity control if the mouse is not turning
    if (cc.turn == 0){
        getDistances(distanceRight, distanceFront, distanceLeft);
        controlStraightVelocityBasedOnDistanceMeasurements();
        outputLeft = pid_velocity_left.output + influenceProximity * pid_distance_wall_left.output;
        outputRight = pid_velocity_right.output + influenceProximity * pid_distance_wall_right.output;
    }
    else{
        outputLeft = pid_velocity_left.output;
        outputRight = pid_velocity_right.output;
    }
    adjustVelocity(outputLeft, outputRight);
    return 0;
}

void adjustVelocity(double outputLeft, double outputRight){
    double velocityLeft = convertOutputToPWMsignal * outputLeft;
    double velocityRight = convertOutputToPWMsignal * outputRight;
    setMotorSpeed(float(velocityLeft), float(velocityRight));
}


void drive_forward_X_cells(double numberOfCells)
{
    initNewControlCycle(1,numberOfCells*cellSize);
}

void drive_backward_X_cells(double numberOfCells)
{
    initNewControlCycle(2,numberOfCells*cellSize);
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
    initNewControlCycle(1,cellSize);
}

void drive_backward()
{
    initNewControlCycle(2,cellSize);
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
