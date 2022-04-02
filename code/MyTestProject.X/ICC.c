/*
 * File:   ICC.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:41
 */


#include "xc.h"
#include "MotorControl.h"
#include "mapping.h"

int positionx = 0;
int positiony = 0;
int orientation = 0; // 0: front, 1: turned to the right, 2: turned to the back, 3: turned to the left
int middlex = 3;
int middley = 3;

int get_positionx()
{
    return positionx;
}

int get_positiony()
{
    return positiony;
}

int get_orientation()
{
    return orientation;
}

// updates the position and orientation after a step
void update_position(char next_step)
{
    if(next_step == 'l')
    {
        if(orientation == 0)
        {
            orientation = 3;
        }
        else if (orientation == 1)
        {
            orientation = 0;
        }
        else if (orientation == 2)
        {
            orientation = 1;
        }
        else // orientation == 3
        {
            orientation = 2;
        }
    }
    else if(next_step == 'r')
    {
        if(orientation == 0)
        {
            orientation = 1;
        }
        else if (orientation == 1)
        {
            orientation = 2;
        }
        else if (orientation == 2)
        {
            orientation = 3;
        }
        else // orientation == 3
        {
            orientation = 0;
        }
    }
    else if(next_step == 'f')
    {
        if(orientation == 0)
        {
            positiony = positiony+1;
        }
        else if (orientation == 1)
        {
            positionx = positionx+1;
        }
        else if (orientation == 2)
        {
            positiony = positiony-1;
        }
        else // orientation == 3
        {
            positionx = positionx-1;
        }
    }
    else if (next_step == 'b')
    {
        if(orientation == 0)
        {
            positiony = positiony-1;
        }
        else if (orientation == 1)
        {
            positionx = positionx-1;
        }
        else if (orientation == 2)
        {
            positiony = positiony+1;
        }
        else // orientation == 3
        {
            positionx = positionx+1;
        }
    }
    else if (next_step == 't')
    {
        if(orientation == 0)
        {
            orientation = 2;
        }
        else if (orientation == 1)
        {
            orientation = 3;
        }
        else if (orientation == 2)
        {
            orientation = 0;
        }
        else // orientation == 3
        {
            orientation = 1;
        }
    }
}

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
    update_position(next_step);
}

void exploring()
{
    while(allexplored()== -1)
    {
        explore(positionx, positiony, orientation);
        char next_step = get_next_step();
        drive(next_step);
        update_map(positionx, positiony, orientation);
        exploring();
    }
}

void drive_to_the_middle()
{
    char next_step;
    set_goal_position(middlex, middley);
    calculateshortestpath(positionx, positiony, orientation);
    // to check if only one more element is in shortest_path
    while(drive_shortest_path()!= -1)
    {
        next_step = get_next_step();
        drive(next_step);
        drive_to_the_middle();
    }
    next_step = get_next_step();
    drive(next_step);
}

void drive_to_the_start()
{
    char next_step;
    set_goal_position(0,0);
    calculateshortestpath(positionx, positiony, orientation);
    // to check if only one more element is in shortest_path
    while(drive_shortest_path()!= -1)
    {
        next_step = get_next_step();
        drive(next_step);
        drive_to_the_middle();
    }
    next_step = get_next_step();
    drive(next_step);
}

void whole_program()
{
    initalize_map();
    exploring();
    drive_to_the_start();
    drive_to_the_middle();
    
}




