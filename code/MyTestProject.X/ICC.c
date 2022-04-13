/*
 * File:   ICC.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 10:41
 */


#include "xc.h"
#include "MotorControl.h"
#include "mapping.h"
#include <stdio.h>
#include <stdbool.h>

int positionx = 0;
int positiony = 0;
int orientation = 0; // 0: front, 1: turned to the right, 2: turned to the back, 3: turned to the left
int middlex = 1;
int middley = 1;
int numDriveInstructions = 0; // Number of drive instructions, such as forward, left, ...
bool compute_path_middle = true;
bool compute_path_start = true;

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
    numDriveInstructions++;
    update_position(next_step);
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

int exploring()
{
    if(allexplored()== -1)
    {
        update_map(positionx, positiony, orientation);
        explore(positionx, positiony, orientation);
        char next_step = get_next_step();
        drive(next_step);
        return 1;
    }
    else{
        update_map(positionx, positiony, orientation);
        //drive_to_the_middle();
    }
    return -1;
}

int drive_to_the_middle()
{
    char next_step;
    if(compute_path_middle)
    {  
        compute_path_middle = false;
        set_goal_position(middlex, middley);
        calculateshortestpath(positionx, positiony, orientation);
          
    }
    //set_goal_position(middlex, middley);
    //calculateshortestpath(positionx, positiony, orientation);
    
    // to check if only one more element is in shortest_path
    
    if(drive_shortest_path()!= -1)
    {
        next_step = get_next_step();
        drive(next_step);
        return 1; 
        //printf("drove %c", next_step);
    }
    else
    {
        return -1;
    }
     
}

int drive_to_the_start()
{
    
    char next_step;
    if(compute_path_start)
    {
        compute_path_start = false;
        set_goal_position(0,0);
        calculateshortestpath(positionx, positiony, orientation);  
//        drive('t');
    }
    // to check if only one more element is in shortest_path
   printf("start\n\r");
   if(drive_shortest_path()!= -1)
    {
       printf("drive start \n\r");
       next_step = get_next_step();
       drive(next_step);
       return 1;
    }
    else
    {
        printf("done\r\n");
        return -1;
    }
      
    return -1;
}

void whole_program()
{
    initalize_map();
    exploring();
    drive_to_the_start();
    drive_to_the_middle();
    
}

void execute_bt_command(char *cmd, char *param) {
    // drive command
    switch (cmd[0]) {
        case 'd':
            // drive command
            drive(param[0]);
            break;
        case 's':
            // stop command
            initNewControlCycle(0, 0);
            break;
    }
}
