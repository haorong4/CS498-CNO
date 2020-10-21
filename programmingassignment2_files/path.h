#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h> 
#include <stdbool.h>
// #include "neighbor.h"

#define LOGFILE "log"
#define TOPOFILE "test_topo.txt"
#define NONE 666

extern int globalMyID;
extern int globalNodeCost[256];
extern int globalNodeNeighbor[256][256];

static int forward_table[256];

int lenC(char** arr);
int lenS(uint16_t* arr);
void destroyC(char** array);
int forward_ID(int dest);
int dijkstra(int src); 
void printSolution(int dist[]);
int minDistance(int dist[], bool sptSet[]) ;

void init_forward_table();
void print_forward_table(int dest);

int forward_ID(int dest){
    if (forward_table[dest] == NONE){
        forward_table[dest] = dijkstra(dest);
    }

    return forward_table[dest];
}

void init_forward_table(){
   for (int i = 0; i < 256; i++){
      forward_table[i] = NONE;
   }
}



int lenS(uint16_t* arr){
   int count = 0;
   while(arr != NULL && arr[count] != NONE){
      count++;
   }
   return count;
}


int minDistance(int dist[], bool sptSet[]) { 
    int min = INT_MAX, min_index; 
    for (int v = 0; v < 256; v++) {
        if (sptSet[v] == false && dist[v] <= min) {
            min = dist[v];
            min_index = v; 
        }
    }
    return min_index; 
} 
  
// A utility function to print the constructed distance array 
void printSolution(int dist[]) { 
    printf("Vertex \t\t Distance from Source\n"); 
    for (int i = 0; i < 256; i++) {
        if (dist[i] == INT_MAX){
            continue;
        }
        printf("%d \t\t %d\n", i, dist[i]); 
    }

} 
  
// Function that implements Dijkstra's single source shortest path algorithm 
// for a graph represented using adjacency matrix representation 
int dijkstra(int src) { 
    int dist[256];
    bool sptSet[256]; 

    for (int i = 0; i < 256; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = false; 
    }
    dist[src] = 0; 
  
    // Find shortest path for all vertices 
    for (int count = 0; count < 256 - 1; count++) {  
        int u = minDistance(dist, sptSet); 
        sptSet[u] = true; 
        for (int v = 0; v < 256; v++) {
            if (!sptSet[v] && globalNodeNeighbor[u][v] && dist[u] != INT_MAX 
                && dist[u] + globalNodeNeighbor[u][v] < dist[v]){
                dist[v] = dist[u] + globalNodeNeighbor[u][v]; 
            }
        }
    } 
    // printSolution(dist);
    int min = INT_MAX;
    int index = NONE;
    for (int i = 0; i < 256; i++) {
        if     (((dist[i] < min) || (dist[i] == min && i < index)) &&
                (dist[i] != INT_MAX) && 
                (i != src) && 
                (i == globalMyID || globalNodeNeighbor[globalMyID][i])){

            min = dist[i];
            index = i;
        } 
    }
    //this means that destination is a neighbor.
    if (index == globalMyID) {index = src;}
    return index;
} 
  