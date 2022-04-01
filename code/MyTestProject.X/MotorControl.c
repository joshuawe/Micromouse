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

const double cellSize = 180;       // length and width of each cell in maze in mm
const double rWheels = 30;         // radius of each wheel in mm
const double aWheels = 102;        // distance between wheels in mm
static double deltaT = 0.1;        // time difference between two measurements
static int encoderCountsPerWheelTurn = 16*1257;

static PIDParameters params = {1.0,1.0,0.0};    // initial control parameters
static double distanceRight, distanceFront, distanceLeft;   // distance measurements
static long countLeft, countRight;                          // encoder counts
static double velocityLeft, velocityRight;                  // to be controlled

static double distanceError = 0.0;
static double integral = 0.0;
static double derivative = 0.0;
static double distanceErrorMemory = 0.0;
static double integralMemory = 0.0;
static double bias = 0.0;
static double output = 0.0;




void initController(double OscillationPeriod)
{
    deltaT = OscillationPeriod;
    params.kP = 1;                      // hint: Ziegler-Nichols method --> kP = 0.45 kU
    params.kI = 1.2*params.kP/deltaT;   // hint: Ziegler-Nichols method --> kP = 1.2 kP/Pu
    params.kD = 0;                      // hint: http://robotsforroboticists.com/pid-control/
}


void getMeasurements(){
    getDistances(distanceRight, distanceFront, distanceLeft);
    getEncoderCounts(countLeft, countRight);
}

int driveStraight(double goalDistance){
    static double currentDistanceDriven = 0.0;
    
    while (currentDistanceDriven <= goalDistance){
        getMeasurements();
        
        distanceError = distanceRight - distanceLeft;
        integral = integralMemory + distanceError * deltaT;
        derivative = (distanceError - distanceErrorMemory) / deltaT;
        output = params.kP * distanceError + params.kI * integral + params.kD * derivative + bias;
        
        adjustLeftVelocity(output);
        adjustRightVelocity(output);
        
        currentDistanceDriven = currentDistanceDriven + countInMM((countLeft + countRight)/2);
        
        distanceErrorMemory = distanceError;
        integralMemory = integral;
    }
    
    return 1;
}

int turnAroundXrad(double angleInRad){
    
    return 1;
}

double countInMM(long count)
{
    double turns = count / encoderCountsPerWheelTurn;
    double mm = turnsInMM(turns);
    return mm;
}

double mmInCount(double mm)
{
    double turns = mmInTurns(mm);
    long count = round(turns*encoderCountsPerWheelTurn);
    return count;
}

double turnsInMM(double turns)
{
    double mm = 2 * PI * turns * rWheels;
    return mm;
}

double mmInTurns(double mm)
{
    double turns = mm / (2 * PI * rWheels);
    return turns;
}

void left_wheel_forward_cm(double turns)
{   
    double mm = turnsInMM(turns);
    double velocity;
}

void left_wheel_backward_cm(double turns)
{
    double mm = turnsInMM(-turns);
    double velocity;
}

void right_wheel_forward_mm(double turns)
{   
    double mm = turnsInMM(turns);
    double velocity;
}

void right_wheel_backward_mm(double turns)
{
    double mm = turnsInMM(-turns);
    double velocity;
}


void drive_forward()
{
    double turns = mmInTurns(cellSize);
    right_wheel_forward_mm(turns);
    left_wheel_forward_mm(turns);
}

void drive_backward()
{
    double turns = mmInTurns(cellSize);
    right_wheel_backward_mm(turns);
    left_wheel_backward_mm(turns);
}


void left_90degree()
{
    int turns= 5; 
    right_wheel_forward_mm(turns);
    left_wheel_backward_mm(turns);
}

void right_90degree()
{
    int turns= 5; 
    right_wheel_backward_mm(turns);
    left_wheel_forward_mm(turns);
}

void turning()
{
    left_90degree();
    left_90degree();
}
