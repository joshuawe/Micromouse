/*
 * File:   pathplanning.c
 * Author: mibet
 *
 * Created on 24. März 2022, 11:46
 */


#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define squares 8;

int goalpositionx = 3;
int goalpositiony = 3;
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

//path: 0 move front, 1 move right, 2 move back, 3 move left
node_t * shortestpath = NULL;
node_t * currentpath = NULL;
int currentdistance;
int mindistance = 10000;


void remove_last(node_t * head) {
    int retval = 0;
    /* wenn nur ein Element in der Liste ist, entferne es */
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



void calculatepath(int positionx, int positiony, cell map[][])
{
    if(positionx == goalpositionx && positiony == goalpositiony && currentdistance < mindistance)
    {
        shortestpath = currentpath; //CHANGE FOR POINTER!
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
                currentdistance++;
                push(currentpath, 0);
                calculatepath(positionx, positiony + 1, map[][]);
            }
            else if(map[positionx][positiony].right == 0)
            {
                currentdistance++;
                push(currentpath, 1);
                calculatepath(positionx+1, positiony, map);
            }
            else if(map[positionx][positiony].back == 0)
            {
                currentdistance++;
                push(currentpath, 2);
                calculatepath(positionx, positiony-1, map);
            }
            else if(map[positionx][positiony].left == 0)
            {
                currentdistance++;
                push(currentpath, 3);
                calculatepath(positionx-1, positiony, map);
            }
            else
            {
                // ERROR STATE
            }
        }
    }
}
node_t * calculateshortestpath(int positionx, int positiony, cell map)
{
    shortestpath = malloc(sizeof(node_t));
    currentpath = malloc(sizeof(node_t));
    calculatepath(positionx, positiony, map);
    return shortestpath;
}