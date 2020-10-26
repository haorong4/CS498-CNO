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

#include "map.h"

#define SEND 1
#define COST 2
#define NEWS 3
#define TOPOFILE "test_topo.txt"

typedef struct _msg_pack {
   char msg[1024];
   int length;
   int dest;
   struct _msg_pack *next;
} msg_pack;

extern int globalMyID;
extern char* logFile;
extern char* costFile;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];

extern int globalNodeNeighbor[256][256];

int originCost[256];


//TODO: add a lock to it;
pthread_mutex_t duck = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t linkMsgLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sendToLock = PTHREAD_MUTEX_INITIALIZER;
msg_pack *channel = NULL;
msg_pack *channel_tail = NULL;

double dropTime = 1200.0;  //700 ms 

void pushMsgChannel(char* content, int dest, int length){
	msg_pack* pack = calloc(1, sizeof(msg_pack));
	memcpy(pack->msg, content, length);
	pack -> dest = dest;
	pack -> length = length;
	pack -> next = NULL;
	pthread_mutex_lock(&duck);
	if (channel == NULL){
		channel = pack;
		channel_tail = pack;
	} else {
		channel_tail -> next = pack;
		channel_tail = pack;
	}
	pthread_mutex_unlock(&duck);
	return;
}

msg_pack* pullMsgChannel(){
	msg_pack* pack = NULL;
	pthread_mutex_lock(&duck);
	if (channel != NULL){
		pack = channel;
		channel = channel -> next;
	}
	pthread_mutex_unlock(&duck);
	return pack;
}


void broadcast(const char* buf, int length)
{
	int i;
	// pthread_mutex_lock(&sendToLock);
	for(i=0;i<256;i++)
		if(i != globalMyID) //(although with a real broadcast you would also get the packet yourself)
			sendto(globalSocketUDP, buf, length, 0,
				  (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
	// pthread_mutex_unlock(&sendToLock);
}

void send_pack(const char* buf, int length, int i)
{
	// pthread_mutex_lock(&sendToLock);
	if(i != globalMyID ) //(although with a real broadcast you would also get the packet yourself)
		sendto(globalSocketUDP, buf, length, 0,
			(struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
	// pthread_mutex_unlock(&sendToLock);
}

double time_diff(struct timeval time1, struct timeval time2){
	double a = (time1.tv_sec - time2.tv_sec) * 1000 + (time1.tv_usec - time2.tv_usec)/1000;
	// a += (time1.tv_usec - time2.tv_usec)/1000;
	return a;
}

int check_neighbor(){
	struct timeval cur_time;
	for(int i = 0; i < 256; i++){
		if(globalNodeNeighbor[globalMyID][i]){
			gettimeofday(&cur_time, 0);
			double temp = time_diff(cur_time, globalLastHeartbeat[i]);
			if (temp > dropTime){
				// TODO: report failure;
				// fprintf(stderr, "Fail to connected with %d", i);
				// log_test("fail!!!!");
				dropLink(i);
			}
		}
	}
	return 0;
}

void* announceToNeighbors(void* unusedParam)
{
	struct timespec sleepLong;
	sleepLong.tv_sec = 0;
	sleepLong.tv_nsec = 300 * 1000 * 1000; //200 ms

	while(1)
	{
		pthread_mutex_lock(&linkMsgLock);
		char* msg = LinkMsg();
		if (msg == NULL){
			msg = "HEREIAM";
		}
		msg = strdup(msg);
		pthread_mutex_unlock(&linkMsgLock);
			
		broadcast(msg, strlen(msg));
		free(msg);
		nanosleep(&sleepLong, 0);
	}
}

void* distributeMessage(void* unusedParam)
{
	struct timespec sleep;
	sleep.tv_sec = 0;
	sleep.tv_nsec = 5 * 1000 * 1000; //5 ms

	while(1)
	{
		msg_pack* pack = pullMsgChannel();
		if (pack == NULL){
			check_neighbor();
			nanosleep(&sleep, 0);
		} else {
			if (pack -> dest != -1){
				send_pack(pack -> msg, pack -> length, pack -> dest);
			} else {
				broadcast(pack -> msg, pack -> length);
			}
			free(pack);
		}
	}
}


void listenForNeighbors()
{
	char fromAddr[100];
	struct sockaddr_in theirAddr;
	socklen_t theirAddrLen;
	unsigned char recvBuf[1000];

	int bytesRecvd;
	while(1)
	{
		theirAddrLen = sizeof(theirAddr);
		if ((bytesRecvd = recvfrom(globalSocketUDP, recvBuf, 1000 , 0, 
					(struct sockaddr*)&theirAddr, &theirAddrLen)) == -1)
		{
			perror("connectivity listener: recvfrom failed");
			exit(1);
		}
		recvBuf[bytesRecvd] = '\0';
		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
		
		short int heardFrom = -1;
		if(strstr(fromAddr, "10.1.1."))
		{
			heardFrom = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			//TODO: this is not very needed, remove it if nothing happens wrong.
			if (linkCost(globalMyID, heardFrom) < 1){
				addLink(heardFrom);
			}
			//record that we heard from heardFrom just now.
			struct timeval ttemp;

			gettimeofday(&ttemp, 0);
			// fprintf(stderr, "time diff between two %d: heart beat: %f ms \n",heardFrom, time_diff(ttemp, globalLastHeartbeat[heardFrom]));
			globalLastHeartbeat[heardFrom] = ttemp;
		}
		// log_test("receive message from someone, starting to break message");
		
		//TODO: use break message here on recvBuf;
		uint16_t task_ID;
		char* task_content;
		char* recvCopy = (char*) calloc(1, bytesRecvd*sizeof(char)+5);
		memcpy(recvCopy, recvBuf, bytesRecvd);

		int mission = breakMessage(recvCopy, &task_ID, &task_content);
		if(mission == SEND)
		{
			if (task_ID == globalMyID){
				log_receive(task_content);
				free(task_content);
				free(recvCopy);
				continue;
			}
			int dest = forward_ID(task_ID);

			if (dest == NONE){
				log_unreachable(task_ID);
				// log_matrix(-1);
				free(task_content);
				free(recvCopy);
				continue;
			}
			pushMsgChannel(recvBuf, dest, bytesRecvd);
			if (heardFrom != -1) {
				log_forward(task_ID, dest, task_content);
			} else {
				log_send(task_ID, dest, task_content);
			}
			// log_matrix(1);
			// log_matrix(4);

		}
		else if(mission == NEWS)
		{
			// log_test(task_content);
			if (updateLinkFromMsg(task_content)){
				pushMsgChannel(recvBuf, -1, bytesRecvd);
			}

		}
		//'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
		else if(mission == COST)
		{
			//TODO: record the cost change (remember, the link might currently be down! in that case,
			//this is the new cost you should treat it as having once it comes back up.)
			// ...
		}
		free(task_content);
		free(recvCopy);
	}
	close(globalSocketUDP);
}




