/*
 * File:   mapping.c
 * Author: mibet
 *
 * Created on 25. Januar 2022, 13:36
 */


/*
 * Map starts always on the left bottom corner
 */

#include "xc.h"
#include <stdbool.h>
#define squares 10

typedef struct {
        int front;
        int back;
        int left;
        int right;             
    }cell;

int maze_size; //in cm
int square_size = 1; //in cm
int position [2];
double front_measurement; //in cm
double right_measurement; // in cm
double left_measurement; // in cm
int i;
int orientation; //front = 0, turned right = 1, back = 2 turned left = 3
bool visited[squares][squares];
cell map[squares][squares];

void set_maze_size(int size)
{
    maze_size = size;
}

void set_square_size(int square)
{
    square_size = square;
}

/*
 * position is set in the size of the maze (e.g. (0,0) is start (0,1) is one step forward)
 */
void set_position(int positionx, int positiony)
{
    position[0] = positionx/square_size;
    position[1] = positiony/square_size;
}

/*
 * Measurement in relation of the square sizes (measurment = 1 -> the obstacle is one square away)
 */
void set_measurements(double new_front_measurement, double new_right_measurement, double new_left_measurement)
{
    front_measurement = (int)new_front_measurement/square_size;
    right_measurement = (int)new_right_measurement/square_size;
    left_measurement = (int)new_left_measurement/square_size;
}

/*
 * free = 0, obstacle  = 1, unsure = -1
 * binary: bottom, left, up, right
 * connections numbers start from left bottom with 0,1,2,3 then going one square up: 
 */
void initalize_map()
{
    int o, j;
    for(o = 0; o < squares; o++)
    {
        for(j=0; j < squares; o++)
        {
            map[o][j].back = -1;
            map[o][j].left = -1;
            map[o][j].right = -1;
            map[o][j].front = -1;
            visited[o][j] = false;
            
        }
    }        
    visited[0][0] = true;
    
   // squares = (int)maze_size/square_size;
    for (i = 0; i < squares; i++)
    {
        map[0][i].back = 1;
        map[i][0].left = 1;
        map[squares][i].right = 1;
        map[i][squares].front = 1;
    }
    
}

void update_map(double new_front_measurement, double new_right_measurement, double new_left_measurement, double new_positionx, double new_positiony)
{
    front_measurement = (int)new_front_measurement/square_size;
    right_measurement = (int)new_right_measurement/square_size;
    left_measurement = (int)new_left_measurement/square_size;
    int positionx = (int) new_positionx/square_size;
    int positiony = (int) new_positiony/square_size;

    if (orientation == 0)
    {
        if (front_measurement >= 2) // only considering the next two squares
        {
            map[positionx][positiony].front = 0;
            map[positionx+1][positiony+1].front = 0;
        }
        else if (front_measurement == 1)
        {
            map[positionx+1][positiony+1].front = 1;
        }
        else if (front_measurement == 0)
        {
            map[positionx][positiony].front = 1;
        }
       if (left_measurement >= 2)
        {
            map[positionx][positiony].left = 0;
            map[positionx+1][positiony+1].left = 0;
        }
       else if (left_measurement == 1)
        {
            map[positionx+1][positiony+1].left = 1;
        }
       else if (left_measurement == 0)
        {
            map[positionx][positiony].left = 1;
        }
        if (right_measurement >= 2)
        {
            map[positionx][positiony].right = 0;
            map[positionx+1][positiony+1].right = 0;
        }
       else if (right_measurement == 1)
        {
            map[positionx+1][positiony+1].right = 1;
        }
       else if (right_measurement == 0)
        {
            map[positionx][positiony].right = 1;
        }
    }
    else  if (orientation == 1) //turned right
    {
        if (front_measurement >= 2) // only considering the next two squares
        {
            map[positionx][positiony].right = 0;
            map[positionx+1][positiony+1].right = 0;
        }
        else if (front_measurement == 1)
        {
            map[positionx+1][positiony+1].right = 1;
        }
        else if (front_measurement == 0)
        {
            map[positionx][positiony].right = 1;
        }
       if (left_measurement >= 2)
        {
            map[positionx][positiony].front = 0;
            map[positionx+1][positiony+1].front = 0;
        }
       else if (left_measurement == 1)
        {
            map[positionx+1][positiony+1].front = 1;
        }
       else if (left_measurement == 0)
        {
            map[positionx][positiony].front = 1;
        }
        if (right_measurement >= 2)
        {
            map[positionx][positiony].back = 0;
            map[positionx+1][positiony+1].back = 0;
        }
       else if (right_measurement == 1)
        {
            map[positionx+1][positiony+1].back = 1;
        }
       else if (right_measurement == 0)
        {
            map[positionx][positiony].back = 1;
        }  
    }
    else if (orientation == 2) // turned back
    {
        if (front_measurement >= 2) // only considering the next two squares
        {
            map[positionx][positiony].back = 0;
            map[positionx+1][positiony+1].back = 0;
        }
        else if (front_measurement == 1)
        {
            map[positionx+1][positiony+1].back = 1;
        }
        else if (front_measurement == 0)
        {
            map[positionx][positiony].back = 1;
        }
       if (left_measurement >= 2)
        {
            map[positionx][positiony].right = 0;
            map[positionx+1][positiony+1].right = 0;
        }
       else if (left_measurement == 1)
        {
            map[positionx+1][positiony+1].right = 1;
        }
       else if (left_measurement == 0)
        {
            map[positionx][positiony].right = 1;
        }
        if (right_measurement >= 2)
        {
            map[positionx][positiony].left = 0;
            map[positionx+1][positiony+1].left = 0;
        }
       else if (right_measurement == 1)
        {
            map[positionx+1][positiony+1].left = 1;
        }
       else if (right_measurement == 0)
        {
            map[positionx][positiony].left = 1;
        }
    }
    else if (orientation == 3 ) //turned left
    {
        if (front_measurement >= 2) // only considering the next two squares
        {
            map[positionx][positiony].left = 0;
            map[positionx+1][positiony+1].left = 0;
        }
        else if (front_measurement == 1)
        {
            map[positionx+1][positiony+1].left = 1;
        }
        else if (front_measurement == 0)
        {
            map[positionx][positiony].left = 1;
        }
       if (left_measurement >= 2)
        {
            map[positionx][positiony].back = 0;
            map[positionx+1][positiony+1].back = 0;
        }
       else if (left_measurement == 1)
        {
            map[positionx+1][positiony+1].back = 1;
        }
       else if (left_measurement == 0)
        {
            map[positionx][positiony].back = 1;
        }
        if (right_measurement >= 2)
        {
            map[positionx][positiony].front = 0;
            map[positionx+1][positiony+1].front = 0;
        }
       else if (right_measurement == 1)
        {
            map[positionx+1][positiony+1].front = 1;
        }
       else if (right_measurement == 0)
        {
            map[positionx][positiony].front = 1;
        }
    }
}

void explore(int positionx, int positiony)
{
    if(explorefront(int positionx, int positiony)== -1)
    {
        if(exploreleft(int positionx, int positiony) == -1)
        {
            if(exploreright(int positionx, int positiony) == -1)
            {
                if(allexplored() == -1)
                {
                    move(int positionx, int positiony);
                }
            }
        }
    }
}

int exploreleft(int positionx, int positiony)
{
    if(positionx == 0 || map[positionx][positiony].left == 1) // if the robot is on the very left of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx-1][positiony] == true) //left square was already visited
    {
        return -1;
    }
    
    //turn to the left and drive one square forward
    return 1;
}

int explorefront(int positionx, int positiony)
{
    if(positionx == squares -1 || map[positionx][positiony].front == 1) // if the robot is on the very top of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx][positiony+1] == true) //left square was already visited
    {
        return -1;
    }
    
    // drive one square forward
    return 1;
}

int exploreright(int positionx, int positiony)
{
    if(positionx == squares -1 || map[positionx][positiony].right == 1) // if the robot is on the very left of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx+1][positiony] == true) //left square was already visited
    {
        return -1;
    }
    
    //turn to the right and drive one step forward
    return 1;
}

int allexplored()
{
    int k,l;
    for(k = 0; k < squares; k++)
    {
        for(l = 0; l < squares; l++)
        {
            if(visited[k][l] == false)
            {
                return -1;
            }
        }
    }
    return 1;
}

void move(int positionx, int positiony)
{
    //try to move somewhere new
}