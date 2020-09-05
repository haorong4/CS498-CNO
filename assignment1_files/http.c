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


#define PORT "3490" // the port client will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 

size_t CONNECT_STATUS = 0;

void *get_in_addr(struct sockaddr *sa);
int client(char* hostname, char* port, char* request);
char** split(char* input, char delimiter);
char** parse_http(char* input);
void free_array_char(char** array );
ssize_t send_to(int sockfd, char* buf, struct addrinfo* p, size_t count);
ssize_t read_from(int sockfd, char* buf, struct addrinfo* p, size_t count);


int main(int argc, char *argv[]){

    if (argc != 2) {
	    fprintf(stderr,"usage: ./http_client http://hostname[:port]/path_to_file\n");
	    exit(1);
    }
    char** request = parse_http(argv[1]);

    client("0.0.0.0", "8000", "GET /test.txt HTTP/1.0");
    return 0;
}

int client (char* hostname, char* port, char* request) {
	int sockfd, numbytes;  
	char buf[1024];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);


    // TODO: sending http request
    int sendbyte = send_to(sockfd, request, p, strlen(request)+1 );

    // TODO: receiving http response
	int readbyte = read_from(sockfd, buf, p, 1024);

    freeaddrinfo(servinfo); // all done with this structure
	close(sockfd);

	return 0;
}

ssize_t send_to(int sockfd, char* buf, struct addrinfo* p, size_t count){
    //TODO: implement function to send message to given socket, in HTTP format.

    size_t bytesTotal = 0; // total number of bytes read
    ssize_t bytesRead = 0; // number of bytes read in one single recv()

    while (bytesTotal < count){
        bytesRead = sendto(sockfd, buf + bytesTotal, count - bytesTotal, 0, p->ai_addr, p->ai_addrlen);
        if (bytesRead == 0){
            fprintf( stderr, "http::send_to connection closed\n");
            return bytesTotal;
        } else if (bytesRead > 0){
            bytesTotal += bytesRead;
        } else {
            fprintf( stderr, "http::send_to error: read failed\n");
            return -1;
        }
    }
    buf[bytesTotal] = '\0';   
    fprintf(stderr, "http::send_to sent '%s'\n",buf);
    return bytesTotal;
}


ssize_t read_from(int sockfd, char* buf, struct addrinfo* p, size_t count){
    //TODO: implement function to receive message from given socket, in HTTP format.
    // int numbytes = -99;
    size_t bytesTotal = 0; // total number of bytes read
    ssize_t bytesRead = 0; // number of bytes read in one single recv()

    while (bytesTotal < count){
        bytesRead = recv(sockfd, buf + bytesTotal, count - bytesTotal, 0);
        if (bytesRead == 0){
            fprintf( stderr, "http::read_from connection closed\n");
            return bytesTotal;
        } else if (bytesRead > 0){
            bytesTotal += bytesRead;
        } else {
            fprintf( stderr, "http::read_from error: read failed\n");
            return -1;
        }
    }
    buf[bytesTotal] = '\0';   
    fprintf(stderr, "http::read_from received '%s'\n",buf);
    return bytesTotal;
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
    char** output = calloc(count+2 , sizeof(char*));
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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void free_array_char(char** array ){
    char** temp = array;
    while(*array != NULL){
        free(*array);
        array += 1;
    }
    free(temp);
}


char** parse_http(char* input){
    if (strlen(input) < 7){
        fprintf(stderr, "INVALIDPROTOCOL" );
    }
    char* example = "http://hostname[:port]/path_to_file";
    char* argument  = strdup(input+7);
    input[7] = '\0';
    char* header = strdup(input);

    if (strcmp(header, "http://") != 0){
        fprintf(stderr, "INVALIDPROTOCOL" );
        free(header);
        free(argument);
        return NULL;
    }
    free(header);

    size_t length = strlen(argument);
    int hostFlag = 1;
    int portFlag = 0;

    ssize_t index1 = -1;
    ssize_t index2 = -1;
    ssize_t check_valid = -2;
    for( size_t i = 0; i < length; i++){
        if ( argument[i] == ':' && (index1 == -1)){
            index1 = i;
        } else if ( argument[i] == '/' && (index2 == -1)){
            index2 = i;
            check_valid = i;
        } else if (argument[i] == '/'){
            if (i == check_valid + 1){
                check_valid = -2;
                break;
            }
            check_valid = i;
        }
    }

    if (index1 >= length || index1 <= 0 || index2 >= length || index2 <= 0 || check_valid < 0) {
        fprintf(stderr, "INVALIDPROTOCOL" );
        free(argument);
        return NULL;
    }

    char** output = calloc(4, sizeof(char*));
    output[0] = strndup(argument, index1);
    output[1] = strndup(argument + index1 + 1, index2-index1-1);
    output[2] = strndup(argument + index2, length - index2);
    output[3] = NULL;

    fprintf(stderr, "good: %s, %s, %s\n", output[0], output[1], output[2]);
    free(argument);
    return output;
}




// ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
//     size_t offset = 0;
//     ssize_t num = 0;
//     while(offset < count){
//       num = read(socket, buffer + offset, count - offset);
//       if(num == 0){
//         return 0;
//       } else if(num > 0){
//         offset += num;
//       } else if(num == -1 && errno == EINTR){
//         continue;
//       } else {
//         return -1;
//       }
//     }
//     return count;
// }

// ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
//   size_t offset = 0;
//   ssize_t num = 0;
//   while(offset < count){
//     num = write(socket, buffer + offset, count - offset);
//     if(num == 0){
//       return 0;
//     } else if(num > 0){
//       offset += num;
//     } else if(num == -1 && errno == EINTR){
//       continue;
//     } else {
//       return -1;
//     }
//   }
//   return count;
// }