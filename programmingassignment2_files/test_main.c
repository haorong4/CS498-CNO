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
#include "map.h"
// #include "path.h"
int globalMyID = 0;
int globalNodeCost[256];
int globalNodeNeighbor[256][256];
int timestamps[256];




// driver program to test above function 
int main() 
{ 
    /* Let us create the example graph discussed above */
    
    initNeighbor();
    init_forward_table();

    // updateLinkFromMsg("link:0:0:1-3,2-2" );
    printLinkMsg();
    updateLinkFromMsg("link:1:0:0-3,3-3,4-4" );
    updateLinkFromMsg("link:2:0:0-2,3-3,5-5" );
    updateLinkFromMsg("link:3:0:1-3,2-3,4-1,5-2" );
    updateLinkFromMsg("link:4:0:1-4,3-1,6-1" );
    updateLinkFromMsg("link:5:0:2-5,3-2,6-2" );
    updateLinkFromMsg("link:6:0:4-1,5-2" );

    printLinkMsg();
     printf("dijkstra: %d\n",dijkstra(4)); 
    printf("destination 1, next stop: %d\n",forward_ID(1));
    printf("destination 4, next stop: %d\n",forward_ID(4)); 
    printf("destination 5, next stop: %d\n",forward_ID(5));


    dropLink(1);
    printLinkMsg();
    printf("destination 1, next stop: %d\n",forward_ID(1));
    printf("destination 4, next stop: %d\n",forward_ID(4)); 
    printf("destination 5, next stop: %d\n",forward_ID(5));

    dropLink(2);
    printLinkMsg();
    printf("destination 1, next stop: %d\n",forward_ID(1));
    printf("destination 4, next stop: %d\n",forward_ID(4)); 
    printf("destination 5, next stop: %d\n",forward_ID(5));
   
  
    return 0; 
} 

// int main(){

//     char* path[10];
//     char* test = "path:9:55-4-5-2-9:566";
//     char* t1 = "path:1:2-44-5-2-1:277";
//     char* t2 = "path:9:2-44-8-2-9:277";
//     char* t3 = "path:4:8-29-9:4";
//     char* t4 = "path:9:55-4-5-2-9:566";
//     char* t5 = "path:77:2-44-5-2-77:77";
//     char* t6 = "path:9:2-44-8-2-9:277";
//     char* t7 = "path:21:8-29-21:21";
//     char* t8 = "path:3:55-4-5-2-3:55";
//     char* t9 = "path:9:2-44-5-2-9:9";

//     path[0] = test;
//     path[1] = t1;
//     path[2] = t2;
//     path[3] = t3;
//     path[4] = t4;
//     path[5] = t5;
//     path[6] = t6;
//     path[7] = t7;
//     path[8] = t8;
//     path[9] = t9;

    
//     for(int i = 0; i< 10000; i++){
//         char buffer[50];
//         int dest = (i * 37 + 2)%256;
//         int cost = (i * 37 + 2) % 1000 + 256;
//         if(i <= 255){
//             dest = i;
//             cost = 0; 
//         }
//         sprintf(buffer, "path:%d:%d-%d-%d-%d-%d:%d", dest, i*17%256,i*19%256, i*29%256, i*2%256, dest, cost );
//         // fprintf(stderr, "%s",buffer);
//         // break;
//         create_path_node(buffer);
//     }

    
//     print_forward_table(-1);


//     return 0;
// }