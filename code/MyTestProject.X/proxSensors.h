/* 
 * File:   proxSensors.h
 * Author: joshu
 *
 * Created on March 23, 2022, 3:04 PM
 */

#ifndef PROXSENSORS_H
#define	PROXSENSORS_H

#define RIGHT_SENSOR 0
#define FRONT_SENSOR 1
#define LEFT_SENSOR 2

extern double distanceLeft;   // [mm] distance from left sensor to wall
extern double distanceFront;  // [mm] distance from front sensor to wall
extern double distanceRight;  // [mm] distance from right sensor to wall

void getDistances(float* distanceLeft, float* distanceFront, float* distanceRight);
int binarySearch(int arr[], int l, int r, int x);
void updateDistances(void);

#endif	/* PROXSENSORS_H */

