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
#include "myTimers.h"
#include "IOconfig.h"
#include "ICC.h"

// Necessary variables
double const distanceTolerance = 80;  // [mm] If a wall is less than distanceTolerance to the side, we take it into account, when controlling the side distance
double toleranceGoal = 3.0;                 // TO CHANGE!!! [mm] tolerated error between goal and current distance
double crossingStartOrEndRecognized = 10.0; // TO CHANGE!!! [mm] difference between two subsequent proximity measurements that indicates a crossing
double toleranceCalibration = 0.5;          // TO CHANGE!!! [mm] tolerated error when calibrating the front distance to the wall
double const influenceProximity = 0.4/80;            // TO CHANGE!!! [rounds/(mm*s)] estimated influence of proximity measurements on control
double desiredTurningSpeed = 0.75;          // TO CHANGE!!! [rounds/s]
double desiredDrivingSpeed = 0.5;           // TO CHANGE!!! [rounds/s]
double maximumOutput = 4;                 // TO CHANGE!!! [rounds/s]

// Controller
static PID_Controller pid_velocity_left;        // Controller for velocity of left wheel
static PID_Controller pid_velocity_right;       // Controller for velocity of right wheel
static PID_Controller pid_distance_wall_left;   // Controller for distance to left wall
static PID_Controller pid_distance_wall_right;  // Controller for distance to right wall


// Control cycle with goals of movement
static Control_Cycle cc;

// Memorized measurements for calibration
static double lastMeasurementDistanceRight;     // [mm] distance from right sensor to wall
static double lastMeasurementDistanceLeft;      // [mm] distance from left sensor to wall
static double lastMeasurementDistanceFront;     // [mm] distance from front sensor to wall

// Distance to goal
double distanceToGoalLeft;           // [mm] distance to goal position of left wheel
double distanceToGoalRight;          // [mm] distance to goal position of right wheel
double distanceToGoal;               // [mm] (distanceToGoalLeft + distanceToGoalRight)/2

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
    
    // Josh Test (frequency = 100 ms)
    init.kP = 0.1;
    init.kI = 0.4;
    init.kD = 0;
    
    // Josh Test (frequency = 20 ms)
    init.kP = 0.35;
    init.kI = 2;
    init.kD = 0.01;
    
    init.integralLimit = 50;              // TO CHANGE!!! limits the integral because the integral slows it down
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
    setControllerParameter(&pid_velocity_left, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);       // TO CHANGE!!!
    setControllerParameter(&pid_velocity_right, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);      // TO CHANGE!!!
    setControllerParameter(&pid_distance_wall_left, init.kFF, init.kP, init.kI, init.kD, init.integralLimit);  // TO CHANGE!!!
    setControllerParameter(&pid_distance_wall_right, init.kFF, init.kP, init.kI, init.kD, init.integralLimit); // TO CHANGE!!!
    
    //setControllerParameter(&pid_velocity_left, 0, 0, 0, 0, init.integralLimit);      // TO CHANGE!!!

    
    // starts with 0 velocity
    adjustVelocity(0,0);
}

/*
 Possibility to tune parameters of the four controller
 * either by editing the values in the call of setControllerParameter in initController
 * or by calling the function (also possible from outside ?!) 
 */
void setControllerParameter(PID_Controller *pid, double kFF, double kP, double kI, double kD, double integralLimit){
    pid->kFF = kFF;
    pid->kP = kP;
    pid->kI = kI;
    pid->kD = kD;
    pid->integralLimit = integralLimit;
}

/*
 Main control step that receives an error and the corresponding pid controller as input and computes the output
 * called by controlBaseVelocity and controlStraightVelocityBasedOnDistanceMeasurements
 */
void controlStep(PID_Controller *pid, double error){
    pid->error = error;
    pid->integral = pid->integralMemory + pid->error * delta_t_sec;
    pid->integral = (pid->integral < pid->integralLimit) ? pid->integral : pid->integralLimit;
    pid->derivative = (pid->error - pid->errorMemory) / delta_t_sec;
    pid->errorMemory = pid->error;
    pid->integralMemory = pid->integral;
    pid->output = pid->kFF * pid->error + pid->kP * pid->error + pid->kI * pid->integral + pid->kD * pid->derivative;
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
    initNewControlCycle(3,PI); // We must use PI for 90 deg, no idea why.
}

void right_90degree()
{
    initNewControlCycle(4,PI);
}

void turning()
{
    initNewControlCycle(3,2*PI);
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

double getErrorFromProximitySensors() {
    double error = 0;
//    LED1 = 0;
//    LED2 = 0;

    // Check, if the walls are ~ equally far away, AND not too far away on either side
    if ((distanceLeft < distanceTolerance) && (distanceRight < distanceTolerance)){
        error = distanceLeft - distanceRight;
//        LED1 = 1;
//        LED2 = 1;


    // Left wall plausibly close, right wall far away
    } else if ((distanceLeft < distanceTolerance) && (distanceRight > distanceTolerance))  {
        error = distanceLeft - DISTANCE_SENSOR_WALL;
//        LED1 = 1;
//        LED2 = 0;

    // Right wall close, left wall far away
    } else if ((distanceRight < distanceTolerance ) && (distanceLeft > distanceTolerance)) {
        error = DISTANCE_SENSOR_WALL - distanceRight;
//        LED1 = 0;
//        LED2 = 1;
    }
    
    // If none of the above is true, error remains zero. (I.e. both walls are far away -> no error)
    
    return error;
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
    int goalReached = checkGoalReachedAlready();
    LED1 = !goalReached;
    
    
    // control of base velocity
    double outputLeft;
    double outputRight;
    
    if (goalReached == 0){
        // use velocity and proximity control if the mouse is not turning
        if (cc.turn == 0){
            calibrateAndControlStraightVelocityBasedOnDistanceMeasurements();
            double error = getErrorFromProximitySensors();
            outputLeft = pid_velocity_left.output - influenceProximity * error;
            outputRight = pid_velocity_right.output + influenceProximity * error;
            
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
 * Helper function to check if the goal position (orientation) is reached already
 * If Goal is reached then return is 1
 * If Goal is not reached then return is 0
 */
int checkGoalReachedAlready()
{
    distanceToGoalLeft = cc.absoluteGoalDistanceLeft - WheelDistanceLeft;
    distanceToGoalRight = cc.absoluteGoalDistanceRight - WheelDistanceRight;
    if (fabs(distanceToGoalLeft) <= toleranceGoal && fabs(distanceToGoalRight) <= toleranceGoal){
        return 1;    // 1 => Goal Reached
    }
    else{
        return 0;    // 0 => Goal Not Reached
    }
}

/*
 Control of velocity based on distance measurement
 * called by executeControl for straight movement only
 */
void calibrateAndControlStraightVelocityBasedOnDistanceMeasurements()
{
    double errorLeft;
    double errorRight;
    static int lastNumDriveInstructions = 0;
    static int wallLeft = 0, wallRight = 0;
          
    controlBaseVelocity();
    
    // Calibration, but Only calibrate if we are not turning
    if (cc.turn == 0) {
        // Calibrate if there is a wall in front
        if ((distanceFront <= DISTANCE_USED_TO_CALIBRATE) && (numDriveInstructions != lastNumDriveInstructions)) { 
            cc.absoluteGoalDistanceLeft  = WheelDistanceLeft  + distanceFront - DISTANCE_FRONT_WALL_STOP;
            cc.absoluteGoalDistanceRight = WheelDistanceRight + distanceFront - DISTANCE_FRONT_WALL_STOP; 
            lastNumDriveInstructions = numDriveInstructions;
        }
    }
    return;
    
    // TODO: Test and re-enable calibration 
    // if wall in front in certain distance, then calibrate 
    if (fabs(distanceFront-DISTANCE_USED_TO_CALIBRATE) < toleranceCalibration){
        calibrateGoalFront();      
    } // else if crossing (=Abzweigung) to left and or right starts or ends
    else if (fabs(lastMeasurementDistanceLeft - distanceLeft) > crossingStartOrEndRecognized
            || fabs(lastMeasurementDistanceRight - distanceRight) > crossingStartOrEndRecognized){
        calibrateGoalSide();
    }
    
    // first: control base velocity with eventually new calibrated goal distances
    controlBaseVelocity();
    // second: control distance to walls
    controlStep(&pid_distance_wall_left,errorLeft);
    controlStep(&pid_distance_wall_right,errorRight);
    
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
 * Function computes the desired velocity of the Mouse in regards to the distance to the goal. 
 * Right now it is quadratic. It could also be linearly decreasing.
 */
double velocityForGoalDistance(double distance, double desiredSpeedMax) {
    double d = 20; // The distance to goal, where we start adjusting
    
    desiredSpeedMax = fabs(desiredSpeedMax);
    
    if (fabs(distance) <= d) {
        d = (desiredSpeedMax / d) * distance;
        return d;                           // linear
        //return (d >= 0) ? d*d : -d*d;       // quadratic (preserving sign)
    } else {
        return (distance >= 0) ? desiredSpeedMax : -desiredSpeedMax;
    }
    

    
}

/*
 Control of base velocity of the two wheels
 * called by executeControl for turning
 * called by calibrateAndControlStraightVelocityBasedOnDistanceMeasurements for straight movement
 */
void controlBaseVelocity()
{    
    double errorSpeedLeft = velocityForGoalDistance(distanceToGoalLeft, cc.desiredSpeedLeft) - speedAngularLeft;
    double errorSpeedRight = velocityForGoalDistance(distanceToGoalRight, cc.desiredSpeedRight) - speedAngularRight;    
    controlStep(&pid_velocity_left,errorSpeedLeft);
    controlStep(&pid_velocity_right,errorSpeedRight);
}


/*
 Helper function to check if the output is too high
 */
double checkOutputIfItExceedsMaximum(double output)
{
    double checkedOutput = 0;
    if (output < 0){                    // drive backwards
        if (-output < maximumOutput){   // output is okay
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


