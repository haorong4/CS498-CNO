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

#include "utils.h"
#include "path.h"

#define LOGFILE "log"
// #define TOPOFILE "test_topo.txt"
// #define COSTFILE "test_cost.txt"

#define NOPASS 0

extern int globalMyID;
extern char* logFile;
extern char* costFile;
extern int globalNodeNeighbor[256][256];
extern int timestamps[256];
extern pthread_mutex_t linkMsgLock;

char* linkMsg = NULL;


int linkCost(int a, int b){
    return globalNodeNeighbor[a][b];
}

void buildLinkMsg(){
    char buf[1024];
    buf[0] = '\0';
    timestamps[globalMyID]++;
    sprintf(buf, "link:%d:%d:", globalMyID,timestamps[globalMyID]);
    for (int i = 0; i < 256; i++){
        int cost = linkCost(globalMyID,i);
        if (cost == 0){
            continue;
        }
        char temp[30];
        temp[0] = '\0';
        // if (i == 255){
        //     sprintf(temp, "%d-%d", i, cost);
        // } else {
        //     sprintf(temp, "%d-%d,", i, cost);
        // }
        sprintf(temp, "%d-%d,", i, cost);
        // log_test(temp);
        strcat(buf, temp);
    }
    if (buf[strlen(buf)-1] == ','){
        buf[strlen(buf)-1] = '\0';
    }
    strcat(buf, ";");
    pthread_mutex_lock(&linkMsgLock);
    if (linkMsg != NULL){
        free(linkMsg);
    }
    linkMsg = strdup(buf);
    pthread_mutex_unlock(&linkMsgLock);
}

void printLinkMsg(){
    printf("Link Message: %s\n", linkMsg);
}
char* LinkMsg(){
    return linkMsg;
}
void updateLink(int a, int b, int cost){
    globalNodeNeighbor[a][b] = cost;
    globalNodeNeighbor[b][a] = cost;
}

// failed to communicate with dest;
void dropLink(int dest){
    updateLink(globalMyID, dest, NOPASS);
    buildLinkMsg();
    init_forward_table();//flush forward table
}

// add a new neigbhor to the map.
void addLink(int dest, int cost){
    updateLink(globalMyID, dest, cost);
    buildLinkMsg();
    init_forward_table();//flush forward table
}


//link:ID:timestamp:p-cost,p-cost,p-cost... 
int updateLinkFromMsg(char* message){
    int ts, src;
    char buf[1024];
    buf[0] = '\0';
    sscanf(message, "link:%d:%d:%s;", &src, &ts, buf);
    if (ts <= timestamps[src] || strlen(buf) <= 1){
        return 0;
    }
    int dirty = linkCost(globalMyID, src);
    timestamps[src] = ts;
    char** links = split(buf, ',');
    int length = lenC(links);
    for (int i = 0; i < 256; i++){
        updateLink(src, i, NOPASS);
    }
    for (int i = 0; i < length; i++){
        int ID, cost;
        sscanf(links[i], "%d-%d", &ID, &cost);
        updateLink(ID, src, cost);
        // printf("add Cost from msg: %d-%d cost=%d\n", src, ID, cost);
    }
    if (dirty != linkCost(globalMyID, src)){
        buildLinkMsg(); 
    } 
    init_forward_table();//flush forward table
    destroyC(links);
    return 1;
}

void initCost(char* _costFile){
    FILE *fp;
    fp = fopen(_costFile, "r+");

    int bytes_read;
    size_t nbytes = 100;
    char *my_string = NULL;
    while ( (bytes_read = getline (&my_string, &nbytes, fp)) != -1) {
        if (my_string[bytes_read-1] == '\n'){
            my_string[bytes_read-1] = '\0';
        }
        int ID;
        int cost;

        sscanf(my_string, "%d %d", &ID, &cost);
        // if (linkCost(globalMyID, ID) != 0){
        updateLink(globalMyID, ID, cost);
        // }
    }
    if (my_string != NULL){
        free(my_string);
    }
    fclose(fp);
}

void initNeighbor(){
    //initiate neighbor array, 1 : yes, 0 : no.
    //TODO: expand this function to figure out cost.
    for (int i = 0; i < 256; i++){
        timestamps[i] = -1;
        for (int j = 0; j < 256; j++){
            globalNodeNeighbor[i][j] = 0;
        }
    }
    // FILE *fp;
    // fp = fopen(TOPOFILE, "r+");

    // int bytes_read;
    // size_t nbytes = 100;
    // char *my_string = NULL;
    // // my_string = (char *) malloc (nbytes + 1);
    // while ( (bytes_read = getline (&my_string, &nbytes, fp)) != -1) {
    //     my_string[bytes_read-1] = '\0';
    //     int ID1;
    //     int ID2;
    //     sscanf(my_string, "%d %d", &ID1, &ID2);
    //     updateLink(ID1, ID2, 1);
    // }
    // if (my_string != NULL){
    //     free(my_string);
    // }
    // fclose(fp);

    initCost(costFile);

    buildLinkMsg();
    timestamps[globalMyID] = -1;
}