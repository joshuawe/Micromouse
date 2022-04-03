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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "proxSensors.h"
#define squares 6

typedef struct {
        int front;
        int back;
        int left;
        int right;             
    }cell;
    
typedef struct node {
    int val;
    struct node * next;
} node_t; 

int goalpositionx = 3;
int goalpositiony = 3;
int maze_size; //in cm
int square_size = 1; //in cm
double front_measurement; //in cm
double right_measurement; // in cm
double left_measurement; // in cm
int i;
char next_step;
bool visited[squares][squares];
cell map[squares][squares];

node_t * shortestpath = NULL;
node_t * currentpath = NULL;
int currentdistance;
int mindistance = 10000;


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
/*void set_position(int positionx, int positiony)
{
    position[0] = positionx/square_size;
    position[1] = positiony/square_size;
}*/

char get_next_step()
{
    return next_step;
}

void set_goal_position(int new_goalpositionx, int new_goalpositiony)
{
    goalpositionx = new_goalpositionx;
    goalpositiony = new_goalpositiony;
}

/*
 * Measurement in relation of the square sizes (measurment = 1 -> the obstacle is one square away)
 */
void set_measurements()
{  
    float new_left_measurement, new_front_measurement, new_right_measurement;
    getDistances(&new_right_measurement, &new_front_measurement, &new_left_measurement);
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

/*
 * add from the new measurements walls in the map
 */
void update_map(int positionx, int positiony, int orientation)
{
    set_measurements(); // actualise measurements

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

/* explore left part of the map
 */
int exploreleft(int positionx, int positiony, int orientation)
{
    if(positionx == 0 || map[positionx][positiony].left == 1) // if the robot is on the very left of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx-1][positiony] == true) //left square was already visited
    {
        return -1;
    }
    
    if(orientation == 0)
    {
        next_step = 'l';
    }
    else if(orientation == 1)
    {
        next_step = 't';
    }
    else if(orientation == 2)
    {
        next_step = 'r'; 
    }
    else
    {
        next_step = 'f';
    }
    return 1;
}

/* explore front part of the map
 */
int explorefront(int positionx, int positiony, int orientation)
{
    if(positionx == squares -1 || map[positionx][positiony].front == 1) // if the robot is on the very top of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx][positiony+1] == true) //left square was already visited
    {
        return -1;
    }
    
    if(orientation == 0)
    {
        next_step = 'f';
    }
    else if(orientation == 1)
    {
        next_step = 'l';
    }
    else if(orientation == 2)
    {
        next_step = 't'; 
    }
    else
    {
        next_step = 'r';
    }
    
    return 1;
}

/* explore right part of the map
 */
int exploreright(int positionx, int positiony, int orientation)
{
    if(positionx == squares -1 || map[positionx][positiony].right == 1) // if the robot is on the very left of the maze or there is a wall on the left
    {
        return -1;
    }    
    else if(visited[positionx+1][positiony] == true) //left square was already visited
    {
        return -1;
    }
    
    if(orientation == 0)
    {
        next_step = 'r';
    }
    else if(orientation == 1)
    {
        next_step = 'f';
    }
    else if(orientation == 2)
    {
        next_step = 'l';
    }
    else
    {
        next_step = 't';
    }
    
    return 1;
}

/* check if every square of the map is explored
 */
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

/* list functions from https://www.learn-c.org/de/Linked%20lists
 * remove the last element from a list
*/
void remove_last(node_t * head) {
    /* if there is only one element */
    if (head->next == NULL) {
        head->val;
        free(head);
        head = NULL;
    }

    node_t * current = head;

    while (current->next->next != NULL) {
        current = current->next;
    }
}

/* remove the first element of a list */
int pop(node_t * head) {
    
    node_t * next_node = NULL;

    if (head == NULL) {
        return -1;
    }

    next_node = (head)->next;
    free(head);
    head = next_node;

    return 1;
}

/* add a value to list
 */
void push(node_t * head, int val) {
    node_t * current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* jetzt koennen wir eine neue Variable hinzufuegen */
    current->next = malloc(sizeof(node_t));
    current->next->val = val;
    current->next->next = NULL;
}

/*replace the values of one list to the other
 */
void replace_list(node_t * current, node_t * old) {

    while (current != NULL && old != NULL) {
        old->val = current->val;
        current = current->next;
        old = old->next;
    }
    //if current is shorter than old, remove the rest of the old list
    while (old->next->next != NULL) {
        old = old->next;
    }
}

/* recursive method to find the shortest path 
 */
void calculatepath(int positionx, int positiony, int orientation)
{
    if(positionx == goalpositionx && positiony == goalpositiony && currentdistance < mindistance)
    {
        replace_list(currentpath, shortestpath); //currentpath will get the shortestpath
        mindistance = currentdistance;
        currentdistance--;
        remove_last(currentpath);
    }
    else
    {
        if(currentdistance < mindistance)
        {
            if(map[positionx][positiony].front == 0)
            {
                if(orientation == 0)
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx, positiony + 1, 0);
                }
                else if (orientation == 1)
                {
                    currentdistance++;
                    push(currentpath, 'l'); // turning to the front
                    calculatepath(positionx, positiony, 0);
                }
                else if (orientation == 2)
                {
                    currentdistance++;
                    push(currentpath, 't');
                    calculatepath(positionx, positiony, 0);
                }
                else
                {
                    currentdistance++;
                    push(currentpath, 'r'); // turning to the front
                    calculatepath(positionx, positiony, 0);
                }
                
            }
            else if(map[positionx][positiony].right == 0)
            {
                if(orientation == 0)
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 1);
                }
                else if (orientation == 1)
                {
                    currentdistance++;
                    push(currentpath, 'f') ;
                    calculatepath(positionx+1, positiony, 1);
                }
                else if (orientation == 2)
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 1);
                }
                else
                {
                    currentdistance++;
                    push(currentpath, 't');
                    calculatepath(positionx, positiony, 1);
                }
            }
            else if(map[positionx][positiony].back == 0)
            {
               if(orientation == 0)
                {
                    currentdistance++;
                    push(currentpath, 't');
                    calculatepath(positionx, positiony, 2);
                }
                else if (orientation == 1)
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 2);
                }
                else if (orientation == 2)
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx, positiony-1, 2);
                }
                else
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 2);
                }
            }
            else if(map[positionx][positiony].left == 0)
            {
               if(orientation == 0)
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 3);
                }
                else if (orientation == 1)
                {
                    currentdistance++;
                    push(currentpath, 't') ;
                    calculatepath(positionx+1, positiony, 3);
                }
                else if (orientation == 2)
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 3);
                }
                else
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx-1, positiony, 3);
                }
            }
            else
            {
                // ERROR STATE
            }
        }
        else 
        {
            currentdistance--;
            remove_last(currentpath);
        }
    }
}

/* calculates the shortest path from the start position to the goalposition
 */
node_t * calculateshortestpath(int positionx, int positiony, int orientation)
{
    //to check: if two times done, is the list newly initalised or the old one still there
    shortestpath = malloc(sizeof(node_t));
    currentpath = malloc(sizeof(node_t));
    calculatepath(positionx, positiony, orientation);
    return shortestpath;
}

int drive_shortest_path()
{
    next_step = (char)shortestpath;
    //check if the last element is removed
    if(pop(shortestpath) == -1)
    {
        return -1;
    }
    return 1;
}

void move(int positionx, int positiony, int orientation)
{
    int k,l;
    for(k = 0; k < squares; k++)
    {
        for(l = 0; l < squares; l++)
        {
            if(visited[k][l] == false)
            {
                set_goal_position(k,l);
                calculateshortestpath(positionx, positiony, orientation);
                drive_shortest_path();       
            }
        }
    }
}

/* explore the whole maze
 */
void explore(int positionx, int positiony, int orientation)
{
    if(explorefront(positionx, positiony, orientation)== -1)
    {
        if(exploreleft(positionx,  positiony, orientation) == -1)
        {
            if(exploreright( positionx,  positiony, orientation) == -1)
            {
                if(allexplored() == -1)
                {
                    move(positionx,  positiony, orientation);
                }
            }
        }
    }
}

node_t * get_shortestpath()
{
    return shortestpath;
}

node_t * get_currentpath()
{
    return currentpath;
}
