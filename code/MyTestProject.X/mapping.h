/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAPPING_H
#define	MAPPING_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define squares 3

typedef struct node {
    char val;
    struct node * next;
} node_t; 

typedef struct {
        int front;
        int back;
        int left;
        int right;             
    }cell;
    
extern cell map[squares][squares];

void set_maze_size(int size);
void set_square_size(int square);
char get_next_step();
int get_next_step_int();
void set_goal_position(int new_goalpositionx, int new_goalpositiony);
node_t * get_shortestpath();
node_t * get_currentpath();
void initalize_map();
void update_map(int positionx, int positiony, int orientation);
int exploreleft(int positionx, int positiony, int orientation);
int explorefront(int positionx, int positiony, int orientation);
int exploreright(int positionx, int positiony, int orientation);
int deadend(int positionx, int positiony, int orientation);
int allexplored();
void move(int positionx, int positiony, int orientation);
void explore(int positionx, int positiony, int orientation);
char remove_last(node_t * head);
int pop(node_t ** head);
void push(node_t * head, char val);
void replace_list(node_t * current, node_t * old);
int undo_last_step(int positionx, int positiony, int orientation);
int calculatepath(int positionx, int positiony, int orientation);
node_t * calculateshortestpath(int positionx, int positiony, int orientation);
int drive_shortest_path();



#endif	/* XC_HEADER_TEMPLATE_H */

