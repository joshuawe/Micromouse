/* 
 * File:   proxSensors.h
 * Author: joshu
 *
 * Created on March 23, 2022, 3:04 PM
 */

#ifndef PROXSENSORS_H
#define	PROXSENSORS_H

#define LEFT_SENSOR 0
#define FRONT_SENSOR 1
#define RIGHT_SENSOR 2


// measurement values for the individual sensors
const int NUM_VALUES = 13;   // number of elements for each array or sub-array 
const int SENSOR_DISTANCES[] = {30, 26, 23, 20, 17, 15, 13, 10, 8, 6, 5, 4, 2};
const int SENSOR_VALUES[3][13] = {{24, 24, 25, 75, 250, 360, 475, 670, 830, 1070, 1280, 1500, 2430},   //left
                             {26, 37, 50, 70, 145, 270, 420, 670, 850, 1120, 1300, 1600, 2700},   // front
                             {23, 23, 40, 150, 305, 420, 505, 675, 830, 1080, 1250, 1500, 2600}}; // right


void getDistance(float* distanceLeft, float* distanceFront, float* distanceRight);
int binarySearch(int arr[], int l, int r, int x);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* PROXSENSORS_H */

