/*
 * File:   MotorControl.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:08
 */


#include "xc.h"


int main(void) {
    return 0;
}


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
    int turns;
    right_wheel_forward_turns(turns);
    left_wheel_forward_turns(turns);
}

void drive_backward()
{
    int turns;
    right_wheel_backward_turns(turns);
    left_wheel_backward_turns(turns);
}

void left_45degree()
{
    int turns; //90°
    right_wheel_forward_turns(turns);
    left_wheel_backward_turns(turns);
}

void right_45degree()
{
    int turns; //90°
    right_wheel_backward_turns(turns);
    left_wheel_forward_turns(turns);
}


void left_90degree()
{
    int turns; //180°
    right_wheel_forward_turns(turns);
    left_wheel_backward_turns(turns);
}

void right_90degree()
{
    int turns; //180°
    right_wheel_backward_turns(turns);
    left_wheel_forward_turns(turns);
}

void turning()
{
    left_90degree();
    left_90degree();
}
