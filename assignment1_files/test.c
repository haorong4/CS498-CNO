#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char** parse_http(char* input);
char** split_body(char* input);

int main(int argc, char *argv[]){

    // 
    // parse_http(argv[1]);

    char* filename = argv[1];
    char* input_line = argv[2];
    
    FILE* file = fopen(filename, "w");

    if (file){
        fwrite(input_line, strlen(input_line), 1, file);
        fwrite("\n", 1, 1, file);
    } else {
        fprintf(stderr, "open file failed\n");
        return -1;
    }

    fclose(file);

    return 0;
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

    ssize_t length = strlen(argument);
    int hostFlag = 1;
    int portFlag = 0;

    ssize_t index1 = -1;
    ssize_t index2 = -1;
    for( size_t i = 0; i < length; i++){
        if ( argument[i] == ':' && (index1 == -1)){
            index1 = i;
        } else if ( argument[i] == '/' && (index2 == -1)){
            index2 = i;
        } 

    }

    if ((index1 >= length) || (index2 >= length) || (index2 <= 0) ) {
        fprintf(stderr, "INVALIDPROTOCOL" );
        free(argument);
        return NULL;
    }

    char** output = (char**)calloc(5, sizeof(char*));       
    if (index1 < 0){
        output[0] = strndup(argument, index2);
        output[1] = strdup("3490");
        output[2] = strndup(argument + index2, length - index2);
    } else {
        output[0] = strndup(argument, index1);
        output[1] = strndup(argument + index1 + 1, index2-index1-1);
        output[2] = strndup(argument + index2, length - index2);
    }
    output[3] = strdup("output");
    output[4] = NULL;

    fprintf(stderr, "good: %s, %s, %s, filename: %s\n", output[0], output[1], output[2], output[3]);
    free(argument);
    return output;
}