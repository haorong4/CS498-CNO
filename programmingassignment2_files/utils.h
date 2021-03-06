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
#define TARGET 1

extern int globalMyID;
extern int globalNodeCost[256];
extern int globalNodeNeighbor[256][256];

extern char* logFile;
extern char* costFile;


int breakMessage(char* message, uint16_t* ID, char** task);
void initCost(char* costFile);
void log_unreachable (uint16_t ID);
void log_send (uint16_t dest, uint16_t next, char* message);
void log_forward (int dest, int next, char* message);
void log_receive (char* message);
char** split(char* input, char delimiter);



int breakMessage(char* message, uint16_t* ID, char** task){
    if (strncmp(message, "link", 4) == 0){
        *task = strdup(message);
        return 3;
    }
    *task = strdup(message+6);
    message[6] = '\0';
    uint16_t temp;  //(uint16_t*) strdup(message+4);
    memcpy (&temp, message+4, 2);

    message[4] = '\0';
    char res[10];
    strcpy(res, message);

    *ID = ntohs(temp);

    if (strcmp(res, "send") == 0){
        return 1;
    } else if (strcmp(res, "cost") == 0){
        return 2;
    } 

    return 0;
}


void log_unreachable (uint16_t ID){
   FILE *fp;

   fp = fopen(logFile, "a+");
   fprintf(fp, "unreachable dest %d\n", ID);
   fclose(fp);
}

void log_send (uint16_t dest, uint16_t next, char* message){
   FILE *fp;

   fp = fopen(logFile, "a+");
   fprintf(fp, "sending packet dest %d nexthop %d message %s\n", dest, next, message);
   fclose(fp);
}

void log_forward (int dest, int next, char* message){
   FILE *fp;

   fp = fopen(logFile, "a+");
   fprintf(fp, "forward packet dest %d nexthop %d message %s\n", dest, next, message);
   fclose(fp);
}

void log_receive (char* message){
   FILE *fp;

   fp = fopen(logFile, "a+");
   fprintf(fp, "receive packet message %s\n", message);
   fclose(fp);
}

void log_test (char* message){
   // if(globalMyID != TARGET){
   //   return;
   // }
   FILE *fp;
   fp = fopen(logFile, "a+");
   fprintf(fp, "log message %s\n", message);
   fclose(fp);
}

void log_matrix (int ID){
   // if(globalMyID != TARGET){
   //   return;
   // }
   if (ID == -1){
      for(int i = 0; i < 256; i++){
         log_matrix(i);
      }
      return;
   }
   FILE *fp;
   fp = fopen(logFile, "a+");
   for (int i = 0; i < 256; i++){
     char temp[20];
     sprintf(temp, " -%d:%d- ", i, globalNodeNeighbor[ID][i]);
     fprintf(fp, "%s", temp);
   }
   fprintf(fp, "%s\n", "");
   fclose(fp);
}


char** split(char* input, char delimiter) {
    int count = 0;
    char* line = strdup(input);
    size_t length = strlen(line);
    if(length == 0){
      return NULL;
    }
    for(size_t i = 0; i < length; i++){
      if(line[i] == delimiter || line[i] == '\n'){
        line[i] = '\0';
        count++;
      }
    }
    char** output = (char**) calloc(count+2 , sizeof(char*));
    int i = 0;
    char* temp = line;
    for(; i < count+1; i++){
      output[i] = strdup(line);
      line += strlen(line) + 1;
    }
    output[i] = NULL;

    free(temp);
    return output;
}


void destroyC(char** array){
    char** temp = array;
    while(*array != NULL){
        free(*array);
        array += 1;
    }
    free(temp);
}

int lenC(char** arr){
   int count = 0;
   while(arr != NULL && arr[count] != NULL){
      count++;
   }
   return count;
}





// int main(int argc, char** argv){

//     // char* str = strdup("cost0155");
//     // char* task;
//     // uint16_t ID = 0;
//     // size_t res = breakMessage(str, &ID, &task);
//     // printf("%zu\n", res);
//     // printf("%d\n", ID);
//     // printf("%s\n", task);

//     // free (task);
//     initCost("log");

//     for (int i = 0; i < 256; i++){
//         printf( "%d: %d\n" , i, globalNodeCost[i]);
//     }
//     return 0;
// }