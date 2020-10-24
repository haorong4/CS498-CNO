#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

// #include "utils.h"
// #include "map.h"
// #include "path.h"
// int globalMyID = 0;
// int globalNodeCost[256];
// int globalNodeNeighbor[256][256];
// int timestamps[256];



int main(){
    char test[] = "hello world";
    test[2] = '\0';
    char* temp = calloc(1,14);
    memcpy(temp, test, 11);
    fprintf(stderr, "%s\n", temp+4);
    free(temp);
    return 0;
}

// // driver program to test above function 
// int main() 
// { 
//     /* Let us create the example graph discussed above */
    
//     initNeighbor();
//     init_forward_table();

//     // updateLinkFromMsg("link:0:0:1-3,2-2" );
//     printLinkMsg();
//     updateLinkFromMsg("link:1:0:0-3,3-3,4-4" );
//     updateLinkFromMsg("link:2:0:0-2,3-3,5-5" );
//     updateLinkFromMsg("link:3:0:1-3,2-3,4-1,5-2" );
//     updateLinkFromMsg("link:4:0:1-4,3-1,6-1" );
//     updateLinkFromMsg("link:5:0:2-5,3-2,6-2" );
//     updateLinkFromMsg("link:6:0:4-1,5-2" );

//     printLinkMsg();
//      printf("dijkstra: %d\n",dijkstra(4)); 
//     printf("destination 1, next stop: %d\n",forward_ID(1));
//     printf("destination 4, next stop: %d\n",forward_ID(4)); 
//     printf("destination 5, next stop: %d\n",forward_ID(5));


//     dropLink(1);
//     printLinkMsg();
//     printf("destination 1, next stop: %d\n",forward_ID(1));
//     printf("destination 4, next stop: %d\n",forward_ID(4)); 
//     printf("destination 5, next stop: %d\n",forward_ID(5));

//     dropLink(2);
//     printLinkMsg();
//     printf("destination 1, next stop: %d\n",forward_ID(1));
//     printf("destination 4, next stop: %d\n",forward_ID(4)); 
//     printf("destination 5, next stop: %d\n",forward_ID(5));
   
  
//     return 0; 
// } 

