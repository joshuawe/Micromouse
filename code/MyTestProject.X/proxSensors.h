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


void getDistances(float* distanceLeft, float* distanceFront, float* distanceRight);
int binarySearch(int arr[], int l, int r, int x);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* PROXSENSORS_H */

