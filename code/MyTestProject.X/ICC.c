/*
 * File:   ICC.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:41
 */


#include "xc.h"
#include "MotorControl.h"

void drive(char next_step)
{
    if(next_step == 'l')
    {
        left_90degree();
    }
    else if(next_step == 'r')
    {
        right_90degree();
    }
    else if(next_step == 'f')
    {
        drive_forward();
    }
    else if (next_step == 'b')
    {
        drive_backward();
    }
    else if (next_step == 't')
    {
        turning();
    }
}

