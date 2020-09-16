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
#define FILE_NOT_FOUND -99


size_t CONNECT_STATUS = 0;

void *get_in_addr(struct sockaddr *sa);
int client(char* hostname, char* port, char* request, char* filename);
char** split(char* input, char delimiter);
char** parse_http(char* input);
char* create_request (char *hostname, char *file_path);
void free_array_char(char** array );
ssize_t send_to(int sockfd, char* buf, struct addrinfo* p, size_t count);
ssize_t read_from(int sockfd, char** buffer, struct addrinfo* p, size_t count);
ssize_t write_to_file(const char* filename, char* buf, size_t count);

void print_array(char** array);
size_t length_array(char** array); 
void destroy_array(char** array);


int main(int argc, char *argv[]){

    if (argc != 2) {
	    fprintf(stderr,"usage: ./http_client http://hostname[:port]/path_to_file\n");
	    exit(1);
    }
    char** request = parse_http(argv[1]);

    if (!request){
        return 0;
    }
    char* http_request = create_request (request[0], request[2]);
    //TODO: build http get request from request array.
    client(request[0], request[1], http_request, "output");

    free(http_request);
    destroy_array(request);
    return 0;
}

int client (char* hostname, char* port, char* request, char* filename) {
	int sockfd, numbytes;  
	char* buf;
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

    // sending http request 
    int sendbyte = send_to(sockfd, request, p, strlen(request)+1 );
    
    // receiving http response
    fprintf(stderr, "HTTP Buffer: %s\n", buf);
	int readbyte = read_from(sockfd, &buf, p, 1024);
    fprintf(stderr, "HTTP Buffer: %s\n", buf);

    // writing buf to file
    write_to_file(filename, buf, readbyte);

    // free(buf);
    freeaddrinfo(servinfo); // all done with this structure
	close(sockfd);

	return 0;
}

ssize_t send_to(int sockfd, char* buf, struct addrinfo* p, size_t count){
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
    // fprintf(stderr, "http::send_to sent '%s'\n",buf);
    return bytesTotal;
}


ssize_t read_from(int sockfd, char** buffer, struct addrinfo* p, size_t count){
    // char header[1024];
    // ssize_t header_length = recv(sockfd, header, count, 0);
    // header[header_length] = '\0';
    // char** headers = split(header, '\n');

    // print_array(headers);
    

    // if (strcmp(headers[0], "HTTP/1.0 200 OK\r")){
    //     fprintf( stderr, "ERROR: %s\n", headers[0]);
    // }
    // size_t content_length;
    // sscanf( headers[4] ,"Content-Length: %zu\r", &content_length); //TODO: possible mistake
    // fprintf(stderr, "Header length: %zd, content_length: %zu\n", header_length, content_length);
    // destroy_array(headers);


    // int numbytes = -99;
    size_t bytesTotal = 0; // total number of bytes read
    ssize_t bytesRead = 0; // number of bytes read in one single recv()
    size_t buffer_szie = 2048;
    char* buf = (char*)calloc(2048, sizeof(char));

    while (1){
        
        bytesRead = recv(sockfd, buf + bytesTotal, count, 0);
        // bytesRead = read(sockfd, buf + bytesTotal, count);
        fprintf(stderr, "bytes Read this turn: %zd", bytesRead);
        if (bytesRead == 0){
            fprintf( stderr, "http::read_from connection closed\n");
            break;
            // return bytesTotal;
        } else if (bytesRead > 0){
            bytesTotal += bytesRead;
            if (bytesTotal >= (buffer_szie - count)){
                buffer_szie *= 2;
                buf = (char*)realloc(buf, buffer_szie);
            }
        } else {
            fprintf( stderr, "http::read_from error: read failed\n");
            return -1;
        }
    }
    buf[bytesTotal] = '\0';   //TODO: MARK for possible segfault;
    fprintf(stderr, "http::read_from received %zu , Buffer Size %zu\n",bytesTotal, buffer_szie);
    *buffer = buf;
    return bytesTotal;
}

ssize_t write_to_file(const char* filename, char* buf, size_t count){

    FILE* file = fopen(filename,"w");

    if (!file){
        fprintf(stderr, "open file failed: %s\n", filename);
        return -1;
    } 

    size_t limit = 1024;
    size_t bytesTotal = 0; // total number of bytes read
    ssize_t bytesWriten = 0; // number of bytes read in one single recv()
    while (bytesTotal < count){
        if ( limit > (count - bytesTotal) ){
            limit = count - bytesTotal;
        }
        bytesWriten = fwrite(buf + bytesTotal, limit, 1, file);
        if (bytesWriten == 0){
            fprintf( stderr, "http::write_to finished\n");
            break;
        } else if (bytesWriten > 0){
            fprintf( stderr, "http::write_to: %zu/%zu\n", bytesTotal, count);
            bytesTotal += limit;
        } else {
            fprintf( stderr, "http::rite_to error: write failed\n");
            return -1;
        }
    }
    fprintf(stderr, "http::Write to total: '%zu'\n",bytesTotal);
    fclose(file);
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
    char** output = (char**)calloc(count+2 , sizeof(char*));
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
        return NULL;
    }
    // char* example = "http://hostname[:port]/path_to_file";
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

    char** output = (char**)calloc(5, sizeof(char*));
    output[0] = strndup(argument, index1);
    output[1] = strndup(argument + index1 + 1, index2-index1-1);
    output[2] = strndup(argument + index2, length - index2);
    output[3] = strndup(argument + check_valid + 1, length - check_valid);
    output[4] = NULL;

    fprintf(stderr, "good: %s, %s, %s, filename: %s\n", output[0], output[1], output[2], output[3]);
    free(argument);
    return output;
}


char* create_request (char *hostname, char *file_path)
{
    char* request = (char*)calloc(1024, sizeof(char));
    char* buf = (char*)calloc(1024, sizeof(char));
    // buffer_appendf(request_buffer, "GET %s HTTP/1.0\r\n", file_path);
    sprintf(buf, "GET %s HTTP/1.0\r\n", file_path);
    strcat(request, buf);
    sprintf(buf, "Host: %s\r\n", hostname);
    strcat(request, buf);
    strcat(request, "Connection: close\r\n\r\n");

    free(buf);
    // fprintf(stderr, "Create Request: %s", request);
    return request;
}


void destroy_array(char** array) {
    char** temp = array;
    while(*array){
        free(*array);
        array += 1;
    }
    free(temp);
    return;
}


size_t length_array(char** array) {
    char** temp = array;
    size_t count = 0;
    while(*temp){
        count += 1;
        temp += 1;
    }
    return count;
}

void print_array(char** array) {
    char** temp = array;
    size_t count = 0;
    fprintf(stderr, "   Print Array:\n");
    while(*temp && count < 20){
        fprintf(stderr, "       index %zu:%s\n", count ,*temp);
        count += 1;
        temp += 1;
    }
    return;
}

