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

#include "util.h"

#define SEND 1
#define COST 2
#define NEWS 3
#define TOPOFILE "test_topo.txt"

typedef struct _msg_pack {
   char msg[1024];
   int length;
   int dest;
   struct _msg_channel *next;
} msg_pack;

extern int globalMyID;
//last time you heard from each node. TODO: you will want to monitor this
//in order to realize when a neighbor has gotten cut off from you.
extern struct timeval globalLastHeartbeat[256];

//our all-purpose UDP socket, to be bound to 10.1.1.globalMyID, port 7777
extern int globalSocketUDP;
//pre-filled for sending to 10.1.1.0 - 255, port 7777
extern struct sockaddr_in globalNodeAddrs[256];

extern int globalNodeNeighbor[256][256];

//TODO: add a lock to it;
msg_pack *channel = NULL;
msg_pack *channel_tail = NULL;

suseconds_t dropTime = 700 * 1000 * 1000;  //700 ms 

// path message format: path:dest:p-p-p-p:cost


//Yes, this is terrible. It's also terrible that, in Linux, a socket
//can't receive broadcast packets unless it's bound to INADDR_ANY,
//which we can't do in this assignment.
void broadcast(const char* buf, int length)
{
	int i;
	for(i=0;i<256;i++)
		if(i != globalMyID && globalNodeNeighbor[globalMyID][i]) //(although with a real broadcast you would also get the packet yourself)
			sendto(globalSocketUDP, buf, length, 0,
				  (struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}

void send_pack(const char* buf, int length, int i)
{
	if(i != globalMyID && globalNodeNeighbor[globalMyID][i]) //(although with a real broadcast you would also get the packet yourself)
		sendto(globalSocketUDP, buf, length, 0,
			(struct sockaddr*)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}

void* announceToNeighbors(void* unusedParam)
{
	struct timespec sleepLong;
	sleepLong.tv_sec = 0;
	sleepLong.tv_nsec = 300 * 1000 * 1000; //300 ms

	struct timespec sleepShort;
	sleepShort.tv_sec = 0;
	sleepShort.tv_nsec = 100 * 1000 * 1000; //300 ms
	while(1)
	{
		if (channel == NULL){
			broadcast("HEREIAM", 7);
			nanosleep(&sleepLong, 0);
		} else {
			msg_pack* pack = channel;
			channel = pack -> next;
			if (pack -> dest != -1){
				send_pack(pack -> msg, pack -> length, pack -> dest);
			} else {
				broadcast(pack -> msg, pack -> length);
				nanosleep(&sleepShort, 0);
			}
		}
	}
}

suseconds_t time_diff(struct timeval time1, struct timeval time2){
	suseconds_t a = (time1.tv_sec - time2.tv_sec) * 1000;
	a += (time1.tv_usec - time2.tv_usec);
	return a;
}

int check_neighbor(){
	struct timeval cur_time;
	for(int i = 0; i < 256; i++){
		if(globalNodeNeighbor[globalMyID][i]){
			gettimeofday(&cur_time, 0);
			if (time_diff(cur_time, globalLastHeartbeat[i]) > dropTime){
				//TODO: report failure;
				removeNeighbor(i);

			}
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
		
		inet_ntop(AF_INET, &theirAddr.sin_addr, fromAddr, 100);
		
		short int heardFrom = -1;
		if(strstr(fromAddr, "10.1.1."))
		{
			heardFrom = atoi(
					strchr(strchr(strchr(fromAddr,'.')+1,'.')+1,'.')+1);
			
			//TODO: this node can consider heardFrom to be directly connected to it; do any such logic now.
			
			//record that we heard from heardFrom just now.
			gettimeofday(&globalLastHeartbeat[heardFrom], 0);
		}
		
		//Is it a packet from the manager? (see mp2 specification for more details)
		//send format: 'send'<4 ASCII bytes>, destID<net order 2 byte signed>, <some ASCII message>
		
		//TODO: use break message here on recvBuf;
		uint16_t task_ID;
		char* task_content;

		int mission = breakMessage(recvBuf, &task_ID, &task_content);
		if(mission == SEND)
		{
			//TODO: send the requested message to the requested destination node
			// ...
		}
		//'cost'<4 ASCII bytes>, destID<net order 2 byte signed> newCost<net order 4 byte signed>
		else if(mission == COST)
		{
			//TODO: record the cost change (remember, the link might currently be down! in that case,
			//this is the new cost you should treat it as having once it comes back up.)
			// ...
		}

		else if(mission == NEWS)
		{
			//TODO: record the cost change (remember, the link might currently be down! in that case,
			//this is the new cost you should treat it as having once it comes back up.)
			// ...
		}

		else if(!strncmp(recvBuf, "pack", 4))
		{
			
		}
		else {
			check_neighbor();
		}
		
		//TODO now check for the various types of packets you use in your own protocol
		//else if(!strncmp(recvBuf, "your other message types", ))
		// ... 
	}
	//(should never reach here)
	close(globalSocketUDP);
}




