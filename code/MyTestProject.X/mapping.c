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
#include "mapping.h"
#define squares 3

typedef struct {
        int front;
        int back;
        int left;
        int right;             
    }cell;
    
/*typedef struct node {
    char val;
    struct node * next;
} node_t; */

int goalpositionx = 1;
int goalpositiony = 1;
int maze_size; //in cm
int square_size = 180; //in mm
int i;
char next_step;
bool visited[squares][squares]; 
cell map[squares][squares];
char last_step;
node_t * shortestpath = NULL;
node_t * currentpath = NULL;
int currentdistance;
int mindistance = 16; // maximal cell number, so that there isn't an endless loop


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

int get_next_step_int()
{
    if(next_step == 'f')
    {
        return 0;
    }
    else if(next_step == 'r')
    {
        return 1;
    }
    else if(next_step == 'b')
    {
        return 2;
    }
    else if(next_step == 'l')
    {
        return 3;
    }
    else if(next_step == 't')
    {
        return 4;
    }
    return -1;
    
}
void set_goal_position(int new_goalpositionx, int new_goalpositiony)
{
    goalpositionx = new_goalpositionx;
    goalpositiony = new_goalpositiony;
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
        for(j=0; j < squares; j++)
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
    if (orientation == 0)
    {
        map[positionx][positiony].front = distanceFront < 100;
        map[positionx][positiony].left = distanceLeft < 100;
        map[positionx][positiony].right = distanceRight < 100;
    }
    else  if (orientation == 1) //turned right
    {
            map[positionx][positiony].right = distanceFront < 100;
            map[positionx][positiony].front = distanceLeft < 100;   
            map[positionx][positiony].back = distanceRight < 100;
           
    }
    else if (orientation == 2) // turned back
    {
            map[positionx][positiony].back = distanceFront <100;
            map[positionx][positiony].right = distanceLeft < 100;
            map[positionx][positiony].left = distanceRight < 100;
    }
    else if (orientation == 3 ) //turned left
    {
            map[positionx][positiony].left = distanceFront < 100;
            map[positionx][positiony].back = distanceLeft < 100;
            map[positionx][positiony].front = distanceRight < 100;
            
        
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
        visited[positionx-1][positiony] = true;
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
        visited[positionx][positiony+1] = true;
        
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
        visited[positionx+1][positiony] = true;
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
char remove_last(node_t * head) {
    /* if there is only one element */
    char last;
    if (head->next == NULL) {
        last = head->val;
        free(head);
        head = NULL;
    }

    node_t * current = head;

    while (current->next->next != NULL) {
        current = current->next;
    }
    return last;
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

void undo_last_step(int positionx, int positiony, int orientation)
{
    last_step = remove_last(currentpath);
    currentdistance--;
    if(orientation == 0)
            {
                if(last_step == 'f')
                {
                    calculatepath(positionx, positiony-1, 0);
                }
                else if(last_step == 'b')
                {
                    calculatepath(positionx, positiony+1,0);
                }
                else if(last_step == 'l')
                {
                    calculatepath(positionx, positiony, 1);
                }
                else if(last_step == 'r')
                {
                    calculatepath(positionx, positiony, 3);
                }
                else
                { /* ERROR*/ }
            }
            else if(orientation == 1)
            {
                if(last_step == 'f')
                {
                    calculatepath(positionx-1, positiony, 1);
                }
                else if(last_step == 'b')
                {
                    calculatepath(positionx+1, positiony,1);
                }
                else if(last_step == 'l')
                {
                    calculatepath(positionx, positiony, 2);
                }
                else if(last_step == 'r')
                {
                    calculatepath(positionx, positiony, 0);
                }
                else
                { /* ERROR*/ }

            }
            else if(orientation == 2)
            {
                if(last_step == 'f')
                {
                    calculatepath(positionx, positiony+1, 2);
                }
                else if(last_step == 'b')
                {
                    calculatepath(positionx, positiony-1,2);
                }
                else if(last_step == 'l')
                {
                    calculatepath(positionx, positiony, 3);
                }
                else if(last_step == 'r')
                {
                    calculatepath(positionx, positiony, 1);
                }
                else
                { /* ERROR*/ }

            }
            else if(orientation == 3)
            {
                if(last_step == 'f')
                {
                    calculatepath(positionx+1, positiony, 3);
                }
                else if(last_step == 'b')
                {
                    calculatepath(positionx-1, positiony,3);
                }
                else if(last_step == 'l')
                {
                    calculatepath(positionx, positiony, 0);
                }
                else if(last_step == 'r')
                {
                    calculatepath(positionx, positiony, 2);
                }
                else
                { /* ERROR*/ }

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
        undo_last_step(positionx, positiony, orientation);
    }
    else
    {
        if(currentdistance < mindistance)
        {
            if(map[positionx][positiony].front == 0)
            {
                if(orientation == 0 && last_step != 'f')
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx, positiony + 1, 0);
                }
                else if (orientation == 1 && last_step != 'l')
                {
                    currentdistance++;
                    push(currentpath, 'l'); // turning to the front
                    calculatepath(positionx, positiony, 0);
                }
                else if (orientation == 2 && last_step != 'b')
                {
                    currentdistance++;
                    push(currentpath, 'b'); //moving backward
                    calculatepath(positionx, positiony+1, 2);
                }
                else if (orientation == 3 && last_step != 'r')
                {
                    currentdistance++;
                    push(currentpath, 'r'); // turning to the front
                    calculatepath(positionx, positiony, 0);
                }
                else 
                {
                    //ERROR
                }
                
            }
            else if(map[positionx][positiony].right)
            {
                if(orientation == 0 && last_step != 'r')
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 1);
                }
                else if (orientation == 1 && last_step != 'f')
                {
                    currentdistance++;
                    push(currentpath, 'f') ;
                    calculatepath(positionx+1, positiony, 1);
                }
                else if (orientation == 2 && last_step != 'l')
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 1);
                }
                else if (orientation == 3 && last_step != 'b')
                {
                    currentdistance++;
                    push(currentpath, 'b');
                    calculatepath(positionx+1, positiony, 3);
                }
                else
                {
                    //ERROR
                }
            }
            else if(map[positionx][positiony].back == 0)
            {
                if(orientation == 0 && last_step != 'b')
                {
                    currentdistance++;
                    push(currentpath, 'b');
                    calculatepath(positionx, positiony-1, 0);
                }
                else if (orientation == 1 && last_step != 'r')
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 2);
                }
                else if (orientation == 2 && last_step != 'f')
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx, positiony-1, 2);
                }
                else if (orientation == 3 && last_step != 'l')
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 2);
                }
                else 
                {
                    //ERROR
                }
            }
            else if(map[positionx][positiony].left == 0)
            {
               if(orientation == 0 && last_step != 'l')
                {
                    currentdistance++;
                    push(currentpath, 'l');
                    calculatepath(positionx, positiony, 3);
                }
               else if (orientation == 1 && last_step != 'b')
                {
                    currentdistance++;
                    push(currentpath, 'b') ;
                    calculatepath(positionx-1, positiony, 1);
                }
               else if (orientation == 2 && last_step != 'r')
                {
                    currentdistance++;
                    push(currentpath, 'r');
                    calculatepath(positionx, positiony, 3);
                }
               else if (orientation == 3 && last_step != 'f')
                {
                    currentdistance++;
                    push(currentpath, 'f');
                    calculatepath(positionx-1, positiony, 3);
                }
               else
               {
                   //ERROR
               }
            }
            else // either walls or already visited cells around the current cell
            {
                undo_last_step(positionx, positiony, orientation);
            }
        }
        else // not possible to reach the goal with less steps than already -> remove last element
        {
            undo_last_step(positionx, positiony, orientation);         
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
    next_step = shortestpath->val;
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
