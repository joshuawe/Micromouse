/*
 * File:   MotorControl.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:08
 */


#include "xc.h"

#define PI = 3.14159265359d;

const double cellSize = 18.0;       // length and width of each cell in maze
const double rWheels = 3.0;         // radius of each wheel in cm
const double aWheels = 10.2;        // distance between wheels in cm
static PIDParameters params = {1.0,1.0,0.0};

void controllerWheelVelocityLeft(double OscillationPeriod){
    initController(double OscillationPeriod);
    
    
}

void initController(double OscillationPeriod)
{
    double Pu = OscillationPeriod;
    params.kP = 1;                  // hint: Ziegler-Nichols method --> kP = 0.45 kU
    params.kI = 1.2*params.kP/Pu;   // hint: Ziegler-Nichols method --> kP = 1.2 kP/Pu
    params.kD = 0;                  // hint: http://robotsforroboticists.com/pid-control/
}

double turnsIncm(double turns)
{
    double cm = 2 * PI * turns * rWheels;
    return cm;
}

double cmInTurns(double cm)
{
    double factor = 2 * PI * rWheels;       // there was an error when I tried to compute turns directly (unexpected token PI)
    double turns = cm / factor;
    return turns;
}

void left_wheel_forward_cm(double turns)
{   
    double cm = turnsIncm(turns);
    double velocity;
}

void left_wheel_backward_cm(double turns)
{
    double cm = turnsIncm(-turns);
    double velocity;
}

void right_wheel_forward_cm(double turns)
{   
    double cm = turnsIncm(turns);
    double velocity;
}

void right_wheel_backward_cm(double turns)
{
    double cm = turnsIncm(-turns);
    double velocity;
}


void drive_forward()
{
    double turns = cmInTurns(cellSize);
    right_wheel_forward_turns(turns);
    left_wheel_forward_turns(turns);
}

void drive_backward()
{
    double turns = cmInTurns(cellSize);
    right_wheel_backward_turns(turns);
    left_wheel_backward_turns(turns);
}

void left_45degree()
{
    int turns = 5; //90°
    right_wheel_forward_turns(turns);
    left_wheel_backward_turns(turns);
}

void right_45degree()
{
    int turns= 5; //90°
    right_wheel_backward_turns(turns);
    left_wheel_forward_turns(turns);
}


void left_90degree()
{
    int turns= 5; //180°
    right_wheel_forward_turns(turns);
    left_wheel_backward_turns(turns);
}

void right_90degree()
{
    int turns= 5; //180°
    right_wheel_backward_turns(turns);
    left_wheel_forward_turns(turns);
}

void turning()
{
    left_90degree();
    left_90degree();
}
