/* This file will handle the proximity sensors from SHARP*/

#include "proxSensors.h"
#include "dma.h"

void getDistances(float* distanceLeft, float* distanceFront, float* distanceRight){
    
    int i, length, pos, posLeft, posRight, value;
    length = NUM_VALUES;
    float* finalDistances[] = {distanceLeft, distanceFront, distanceRight};
    
    // Loop through all sensors from left to front to right
    for (i=0; i<3; i++) {
        value = adcData[i];   // read out the data from the sensors
                
        // use binarySearch to find closest element
        pos = binarySearch(SENSOR_VALUES[i], 0, length-1, value);    
    
        // check if pos is positioned at the left or right boundary of array
        if (pos <= 0) {
            posLeft = 0;
            posRight = 1;
        }
        else if (pos >= length-1) {
            posLeft = length-2;
            posRight = length-1;
        }
        else {
            posLeft = pos;
            posRight = pos + 1;
        }

        // interpolation -> yInterpolated = y0 + ((y1-y0)/(x1-x0)) * (xp - x0);
        float yInterpolated;
        int xLeft = SENSOR_VALUES[i][posLeft];
        int xRight = SENSOR_VALUES[i][posRight];
        int yLeft = SENSOR_DISTANCES[posLeft];
        int yRight = SENSOR_DISTANCES[posRight];
        yInterpolated = yLeft + ((float)(yRight-yLeft)/ (float)(xRight- xLeft)) * (value - xLeft);
    
        *(finalDistances[i]) = yInterpolated;
    }  
    
}


/* binarySearch(array, left, right, search)
    Args:
        arr array: the array, where to search.
        l   left: index from where to start seaching in array.
        r   right: index until where to search in array. (Remember 0-indexing.)
        x   search: The value you are looking for.
        
    Note: 
        - If element is not in the array, then the result will be the next smaller element closest to the value.
        - If element is smaller than smallest value then it will return array[first].
        - If element is greater than greatest value then it will return array[last].
*/
int binarySearch(int arr[], int l, int r, int x)
{
    if (r >= l) {
        int mid = l + (r - l) / 2;
 
        // If the element is present at the middle
        // itself
        if (arr[mid] == x)
            return mid;
 
        // If element is smaller than mid, then
        // it can only be present in left subarray
        if (arr[mid] > x)
            return binarySearch(arr, l, mid - 1, x);
 
        // Else the element can only be present
        // in right subarray
        return binarySearch(arr, mid + 1, r, x);
    }
 
    // We reach here when element is not
    // present in array
    //return -1;
    
    // if element is greater than greatest
    if (arr[r] < x) return r;
    else return l;
}