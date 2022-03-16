/*
 * File:   MotorControl.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:08
 */


#include "xc.h"



void left_wheel_forward_turns(int turns)
{   
}

void left_wheel_backward_turns(int turns)
{
}

void right_wheel_forward_turns(int turns)
{
}

void right_wheel_backward_turns(int turns)
{
}


void drive_forward()
{
    int turns = 5;
    right_wheel_forward_turns(turns);
    left_wheel_forward_turns(turns);
}

void drive_backward()
{
    int turns = 5;
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
