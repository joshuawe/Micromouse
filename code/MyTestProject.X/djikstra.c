/*
 * File:   calculatepath.c
 * Author: mibet
 *
 * Created on 24. März 2022, 10:32
 */


#include "xc.h"
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
   

#define squares 8*8
   
// A utility function to find the 
// sqaure with minimum distance
// value, from the set of squares
// not yet included in shortest
// path tree
int minDistance(int dist[], 
                bool found_shortest_dist[])
{
       
    // Initialize min value
    int min = INT_MAX, min_index;
    int v;
    for (v = 0; v < squares; v++)
        if (found_shortest_dist[v] == false &&
                   dist[v] <= min)
            min = dist[v], min_index = v;
   
    return min_index;
}
   
// Function to print shortest
// path from source to j
// using parent array
void printPath(int parent[], int j)
{
       
    // Base Case : If j is source
    if (parent[j] == - 1)
        return;
   
    printPath(parent, parent[j]);
   
    printf("%d ", j);
}
   
// A utility function to print 
// the constructed distance
// array
int printSolution(int dist[], int n, 
                      int parent[])
{
    int src = 0;
    int i;
    printf("Square\t Distance\tPath");
    for ( i = 1; i < squares; i++)
    {
        printf("\n%d -> %d \t\t %d\t\t%d ",
                      src, i, dist[i], src);
        printPath(parent, i);
    }
}
   
// Function that implements Dijkstra's
// single source shortest path
// algorithm for a graph represented
// using adjacency matrix representation
void dijkstra(int graph[squares][squares], int src)
{
       
    // The output array. dist[i]
    // will hold the shortest
    // distance from src to i
    int dist[squares]; 
   
    // found_shortest_dist[i] will true if squares
    // i is included / in shortest
    // path tree or shortest distance 
    // from src to i is finalized
    bool found_shortest_dist[squares];
   
    // Parent array to store
    // shortest path tree
    int parent[squares];
    int i;
    // Initialize all distances as 
    // INFINITE and stpSet[] as false
    for ( i = 0; i < squares; i++)
    {
        parent[0] = -1;
        dist[i] = INT_MAX;
        found_shortest_dist[i] = false;
    }
   
    // Distance of source squares 
    // from itself is always 0
    dist[src] = 0;
    int count;
    // Find shortest path
    // for all squares
    for ( count = 0; count < squares - 1; count++)
    {
        // Pick the minimum distance
        // square from the set of
        // squares not yet processed. 
        // u is always equal to src
        // in first iteration.
        int u = minDistance(dist, found_shortest_dist);
   
        // Mark the picked vertex 
        // as processed
        found_shortest_dist[u] = true;
        int v;
        // Update dist value of the 
        // adjacent squares of the
        // picked vertex.
        for ( v = 0; v < squares; v++)
   
            // Update dist[v] only if is
            // not in found_shortest_dist, there is
            // an edge from u to v, and 
            // total weight of path from
            // src to v through u is smaller
            // than current value of
            // dist[v]
            if (!found_shortest_dist[v] && graph[u][v] &&
                dist[u] + graph[u][v] < dist[v])
            {
                parent[v] = u;
                dist[v] = dist[u] + graph[u][v];
            } 
    }
   
    // print the constructed
    // distance array
    printSolution(dist, squares, parent);
}
