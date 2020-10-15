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

#define LOGFILE "log"
#define TOPOFILE "test_topo.txt"
#define NONE 666

extern int globalMyID = 0;

typedef struct Path {
   int cost;
   uint16_t *route;
   uint16_t pathLen;
   struct Path *next; 
   struct Path *prev;
}path;

extern int globalMyID = 0;
extern int globalNodeCost[256];
extern int globalNodeNeighbor[256][256];

path* forward_table[256];

int len(char** arr);
int len(uint16_t* arr);
int compare(path* node1, path* node2);
void destroy(path* node);
void destroy(char** array);
int forward_ID(int dest);
char* generate_path_info(int dest);
void create_path_node(char* path_message);
void remove_path_node(path* node, int dest);
void insert_path_node(path* node, int dest);
void init_forward_table();

int forward_ID(int dest){
   return forward_table[dest] -> route[0];
}

// path:dest:p-p-p-p-p:cost
char* generate_path_info(int dest){
   path* p = forward_table[dest];
   char message[1024];
   char buffer[50];
   message[0] = '\0';
   strcat(message, "path:");
   sprintf(buffer,"%d:%d",dest, globalMyID);
   strcat(message, buffer);

   for (int i = 0; i < p->pathLen; i++){
      char b[6];
      sprintf(b,"-%d",p->route[i]);
      strcat(message, b);
   }
   buffer[0] = '\0';
   sprintf(buffer,":%d", p->cost);
   strcat(message, buffer);
   char* ret = strdup(message);
   return ret;
}
// path:dest:p-p-p-p-p:cost
void create_path_node(char* path_message){

   char** messages = split(path_message, ':');

   path* node = (path*)calloc(1, sizeof(path));
   node -> cost = atoi(messages[3]);
   node -> next = NULL;
   node -> prev = NULL;

   char** route = split(messages[2], '-');
   int route_len = len(route);
   node -> pathLen = route_len;
   node -> route = (uint16_t*)calloc(route_len+1, sizeof(uint16_t));
   node -> route[route_len] = NONE;
   for (int i = 0; i < route_len; i++){
      node -> route[i] = atoi(route[i]);
      i++;
   }

   insert_path_node(node, atoi(messages[1]));
   destroy(messages);
   destroy(route);

   return;
}

void remove_path_node(path* node, int dest){
   path* head = forward_table[dest];

   if (node -> prev != NULL){
      node -> prev -> next = node -> next;
   }
   if (node -> next != NULL){
      node -> next -> prev = node -> prev;
   }
   if (head == node){
      head = node -> next;
   }
   forward_table[dest] = head;
   node -> next = NULL;
   node -> prev = NULL; 
   destroy(node);
   return;
}

void insert_path_node(path* node, int dest){
   path* head = forward_table[dest];
   
   path* cur = head;
   while (cur != NULL){
      if (compare(node, cur)){
         if (head == cur){
            node -> next = cur;
            cur -> prev = node;
            head = node;
            break;
         }
         if (cur -> prev != NULL){
            cur -> prev -> next = node;
         }
         node -> prev = cur -> prev;
         node -> next = cur;
         cur -> prev = node;
         break;
      }
      cur = cur -> next;
   }
   forward_table[dest] = head;
   return;
}

void init_forward_table(){
   for (int i = 0; i < 256; i++){
      forward_table[i] = NULL;
   }
}

int len(char** arr){
   int count = 0;
   while(arr != NULL && arr[count] != NULL){
      count++;
   }
   return count;
}

int len(uint16_t* arr){
   int count = 0;
   while(arr != NULL && arr[count] != NONE){
      count++;
   }
   return count;
}

// compare two path cost, tie-breaking included, 
// 1: node1 cost less than node2;
// 0: node1 cost more than node2;
int compare(path* node1, path* node2){
   if (node1 -> cost > node2 -> cost){
      return 0;
   }
   if (node1 -> cost < node2 -> cost){
      return 1;
   }

   int len1 = node1 -> pathLen;
   int len2 = node2 -> pathLen;
   for (int i = 0; i < len1 && i < len2; i++){
      if (node1 -> route[i] > node2 -> route[i]){
         return 0;
      }
      if (node1 -> route[i] < node2 -> route[i]){
         return 1;
      }
   }
   if (len1 < len2){
      return 1;
   }
   return 0;
}

void destroy(path* node){
   free (node->route);
   free (node);
   return;
}

void destroy(char** array){
    char** temp = array;
    while(*array != NULL){
        free(*array);
        array += 1;
    }
    free(temp);
}