/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>

int distances[14] = {30, 26, 23, 20, 17, 15, 13, 10, 8, 6, 5, 4, 2};
int values[14] = {24, 24, 25, 75, 250, 360, 475, 670, 830, 1070, 1280, 1500, 2430};

/* binarySearch(array, left, right, search)
    Args:
        array: the array, where to search.
        left: index from where to start seaching in array.
        right: index until where to search in array. (Remember 0-indexing.)
        Search: The value you are looking for.
        
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
    

float getDistance(float value2, int sensorDistances[], int sensorValues[]) {
    int length, pos, posLeft, posRight;
    
    float value = 999;
    
    // use binarySearch to find closest element
    length = sizeof(values)/sizeof(values[0]);
    pos = binarySearch(values, 0, length-1, value);
    
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
    int xLeft = sensorValues[posLeft];
    int xRight = sensorValues[posRight];
    int yLeft = sensorDistances[posLeft];
    int yRight = sensorDistances[posRight];
    yInterpolated = yLeft + ((float)(yRight-yLeft)/ (float)(xRight- xLeft)) * (value - xLeft);
    printf("yInterpolated %f\n", yInterpolated);
    
    printf("posLeft %d, pos %d, posRight %d\n", posLeft, pos, posRight);
    printf("sensorValueLeft %d, value %f, sensorValueRight %d;\tdistanceLeft %d, interpolation %.3f, distanceRight %d\n", 
            xLeft, value, xRight, yLeft, yInterpolated, yRight);
    return yInterpolated;
    
    
    
    
}



int main()
{
    printf("Hello World!\n");
    int length = sizeof(distances)/sizeof(distances[0]);
    printf("The array has %d elements.\n", length);
    
    float search = 5.5;
    int found = binarySearch(distances, 0, length-1, search);
    printf("Searching for %f in distances. Found at position %d, value=%d\n", search, found, distances[found]);
    
    
    float interpolated = getDistance(1200, distances, values);
    
    
}
