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
#include "math.h"

// Necessary variables
extern int delta_t_timer;
double toleranceGoal = 2.0;                 // TO CHANGE!!! [mm] tolerated error between goal and current distance
double crossingStartOrEndRecognized = 10.0; // TO CHANGE!!! [mm] difference between two subsequent proximity measurements that indicates a crossing
double toleranceCalibration = 0.5;          // TO CHANGE!!! [mm] tolerated error when calibrating the front distance to the wall
double influenceProximity = 0.3;            // TO CHANGE!!! [rounds/(mm*s)] estimated influence of proximity measurements on control
double desiredTurningSpeed = 0.25;          // TO CHANGE!!! [rounds/s]
double desiredDrivingSpeed = 1.5;           // TO CHANGE!!! [rounds/s]
double maximumOutput = 1.0;                 // TO CHANGE!!! [rounds/s]

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
static double measurementDistanceRight;     // [mm] distance from right sensor to wall
static double measurementDistanceLeft;      // [mm] distance from left sensor to wall
static double measurementDistanceFront;     // [mm] distance from front sensor to wall

// Memorized measurements for calibration
static double lastMeasurementDistanceRight;     // [mm] distance from right sensor to wall
static double lastMeasurementDistanceLeft;      // [mm] distance from left sensor to wall
static double lastMeasurementDistanceFront;     // [mm] distance from front sensor to wall

// Distance to goal
static double distanceToGoalLeft;           // [mm] distance to goal position of left wheel
static double distanceToGoalRight;          // [mm] distance to goal position of right wheel
static double distanceToGoal;               // [mm] (distanceToGoalLeft + distanceToGoalRight)/2

Outputs interestingOutputForDebugging(){
    Outputs out;
    out.controlCycle = cc;
    out.velocity_left_error = pid_velocity_left.error;
    out.velocity_left_output= pid_velocity_left.output;
    out.velocity_right_error = pid_velocity_right.error;
    out.distance_wall_right_output = pid_velocity_right.output;
    out.distance_wall_left_error = pid_distance_wall_left.error;
    out.distance_wall_left_output = pid_distance_wall_left.output;
    out.distance_wall_right_error = pid_distance_wall_right.error;
    out.distance_wall_right_output = pid_distance_wall_right.output;
    out.distance_to_goal_left = distanceToGoalLeft;
    out.distance_to_goal_right = distanceToGoalRight;
    out.distance_to_goal = distanceToGoal;
    return out;
}

/*****************************************************************************
 *                                                                           * 
 *   PART 0 - Initialization of Controller and general control functions     *
 *                                                                           *
 *****************************************************************************/

/*
 Entry point 0 for highlevel code
 * Initialization of controller -> called 1 time before program starts
 * sets velocity of motors to 0
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
    
    // starts with 0 velocity
    adjustVelocity(0,0);
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





/*****************************************************************************
 *                                                                           * 
 *     PART 1 - Initialization of new control cycle by highlevel control     *
 *                                                                           *
 *****************************************************************************/




/*
 Entry point 1 for highlevel code
 * initializes controller for this control cycle depending on called function:
 * not possible - drive_forward_X_cells -> input: double numberOfCells
 * not possible - drive_backward_X_cells -> input: double numberOfCells
 * left_X_rad -> input: angleInRad
 * right_X_rad -> input: angleInRad
 * drive_forward
 * drive_backward
 * left_90degree
 * right_90degree
 * turn_around 
 */
//void drive_forward_X_cells(double numberOfCells) // can not be used anymore otherwise calibration needs to be adjusted
//{
//   initNewControlCycle(1,numberOfCells*CELL_SIZE);
//}
//
//void drive_backward_X_cells(double numberOfCells) // can not be used anymore otherwise calibration needs to be adjusted
//{
//    initNewControlCycle(2,numberOfCells*CELL_SIZE);
//}

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
    // reset of velocity controller memories that include values of former tasks
    pid_velocity_left.errorMemory = 0;
    pid_velocity_right.errorMemory = 0;
    pid_velocity_left.integralMemory = 0;
    pid_velocity_right.integralMemory = 0;
}





/*****************************************************************************
 *                                                                           * 
 *         PART 2 - Execution of control cycle including calibration         *
 *                                                                           *
 *****************************************************************************/



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
    int goalReached = checkGoalReachedAlready();
    
    
    // control of base velocity
    double outputLeft;
    double outputRight;
    
    if (goalReached == 0){
        // use velocity and proximity control if the mouse is not turning
        if (cc.turn == 0){
            getMeasurements();
            calibrateAndControlStraightVelocityBasedOnDistanceMeasurements();
            outputLeft = pid_velocity_left.output + influenceProximity * pid_distance_wall_left.output;
            outputRight = pid_velocity_right.output + influenceProximity * pid_distance_wall_right.output;
        }
        else{ // use only velocity control if the mouse is turning
            controlBaseVelocity();
            outputLeft = pid_velocity_left.output;
            outputRight = pid_velocity_right.output;  
        }
        outputLeft = checkOutputIfItExceedsMaximum(outputLeft);
        outputRight= checkOutputIfItExceedsMaximum(outputRight);
        adjustVelocity(outputLeft, outputRight); 
    }
    else{
        adjustVelocity(0,0);
    }
    return goalReached;
}

/*
 Helper function to check if the goal position (orientation) is reached already
 */
int checkGoalReachedAlready()
{
    distanceToGoalLeft = cc.absoluteGoalDistanceLeft - WheelDistanceLeft;
    distanceToGoalRight = cc.absoluteGoalDistanceRight - WheelDistanceRight;
    distanceToGoal = (distanceToGoalLeft + distanceToGoalRight) / 2;
    if (fabs(distanceToGoal) <= toleranceGoal){
        return 1;
    }
    else{
        return 0;
    }
}

/*
 Input measurements of distance sensors
 * write pointer value in local variable
 */
void getMeasurements(){
    getDistances(distanceRight, distanceFront, distanceLeft);
    lastMeasurementDistanceRight = measurementDistanceRight;
    lastMeasurementDistanceLeft = measurementDistanceLeft;
    lastMeasurementDistanceFront = measurementDistanceFront;
    measurementDistanceRight = *distanceRight;
    measurementDistanceLeft = *distanceLeft;
    measurementDistanceFront = *distanceFront;
}


/*
 Control of velocity based on distance measurement
 * called by executeControl for straight movement only
 */
void calibrateAndControlStraightVelocityBasedOnDistanceMeasurements()
{
    double errorLeft;
    double errorRight;
          
    // no walls to both sides -> just continue with base velocity
    if(measurementDistanceLeft > MAX_POSS_DISTANCE_SENSOR_WALL && measurementDistanceRight > MAX_POSS_DISTANCE_SENSOR_WALL){
        controlBaseVelocity();
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
    // walls to both sides
    else{
        errorLeft = DISTANCE_SENSOR_WALL - measurementDistanceLeft;
        errorRight = DISTANCE_SENSOR_WALL - measurementDistanceRight;
    }
    
    // if wall in front in certain distance, then calibrate 
    if (fabs(measurementDistanceFront-DISTANCE_USED_TO_CALIBRATE) < toleranceCalibration){
        calibrateGoalFront();      
    } // else if crossing to left and or right starts or ends
    else if (fabs(lastMeasurementDistanceLeft - measurementDistanceLeft) > crossingStartOrEndRecognized
            || fabs(lastMeasurementDistanceRight - measurementDistanceRight) > crossingStartOrEndRecognized){
        calibrateGoalSide();
    }
    
    // first: control base velocity with eventually new calibrated goal distances
    controlBaseVelocity();
    // second: control distance to walls
    controlStep(pid_distance_wall_left,errorLeft);
    controlStep(pid_distance_wall_right,errorRight);
    
}

/*
 Calibration if corridor to the side when driving forward
 */
void calibrateGoalSide(){
    // do not calibrate if moving backwards or turning
    if (cc.desiredSpeedLeft > 0 && cc.desiredSpeedRight > 0){
        cc.absoluteGoalDistanceLeft = CALIBRATION_SIDE;
        cc.absoluteGoalDistanceRight = CALIBRATION_SIDE;
    }
}

/*
 Calibration if wall in front when driving forward
 */
void calibrateGoalFront(){
    if (cc.desiredSpeedLeft > 0 && cc.desiredSpeedRight > 0){
        cc.absoluteGoalDistanceLeft = CALIBRATION_FRONT;
        cc.absoluteGoalDistanceRight = CALIBRATION_FRONT;
    }
}


/*
 Control of base velocity of the two wheels
 * called by executeControl for turning
 * called by calibrateAndControlStraightVelocityBasedOnDistanceMeasurements for straight movement
 */
void controlBaseVelocity()
{    
    double errorSpeedLeft = cc.desiredSpeedLeft - speedAngularLeft;
    double errorSpeedRight = cc.desiredSpeedRight - speedAngularRight;    
    controlStep(pid_velocity_left,errorSpeedLeft);
    controlStep(pid_velocity_right,errorSpeedRight);
}


/*
 Helper function to check if the output is too high
 */
double checkOutputIfItExceedsMaximum(double output)
{
    double checkedOutput = 0;
    if (output < 0){                    // drive backwards
        if (output > -maximumOutput){   // output is okay
            checkedOutput = output;
        }
        else{                           // output is too high (too large negative number)
            checkedOutput = -maximumOutput;
        }
    }
    else{                               // drive forwards
        if (output < maximumOutput){    // output is okay  
            checkedOutput = output;
        }
        else{                           // output is too high (too large positive number)
            checkedOutput = maximumOutput;
        }
    }
    return checkedOutput;
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


