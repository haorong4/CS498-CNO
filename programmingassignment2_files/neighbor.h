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

#define LOGFILE "log"
#define TOPOFILE "test_topo.txt"

extern int globalMyID = 0;
extern int globalNodeCost[256];
extern int globalNodeNeighbor[256][256];

void addNeighbor(int ID){
    globalNodeNeighbor[globalMyID][ID] = 1;
    globalNodeNeighbor[ID][globalMyID] = 1;
}

void removeNeighbor(int ID){
    globalNodeNeighbor[globalMyID][ID] = 0;
    globalNodeNeighbor[ID][globalMyID] = 0;
}

void initNeighbor(){
    //initiate neighbor array, 1 : yes, 0 : no.
    //TODO: expand this function to figure out every neigbhor.
    for (int i = 0; i < 256; i++){
        for (int j = 0; j < 256; j++){
            globalNodeNeighbor[i][j] = 0;
        }
    }

    FILE *fp;
    fp = fopen(TOPOFILE, "r+");

    int bytes_read;
    size_t nbytes = 100;
    char *my_string = NULL;
    // my_string = (char *) malloc (nbytes + 1);
    while ( (bytes_read = getline (&my_string, &nbytes, fp)) != -1) {
        my_string[bytes_read-1] = '\0';
        int ID1;
        int ID2;

        sscanf(my_string, "%d %d", &ID1, &ID2);
        globalNodeNeighbor[ID1][ID2] = 1;
        globalNodeNeighbor[ID2][ID1] = 1;
    }
    if (my_string != NULL){
        free(my_string);
    }
    fclose(fp);

}