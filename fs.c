#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_error(char *msg){
    perror(msg);
    exit(1);
}

FileSystem* init_fs(int buff_size){
    
    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if(!fs){
        handle_error("Error in allocating memory for file system");
        return NULL;
    }

    fs->buff = (char*)malloc(buff_size);
    if(!fs->buff){
        handle_error("Error in allocating memory for buffer");
        return NULL;
    }

    fs->buff_size = buff_size;
    fs->curr_directory.elements = (DirectoryElement*)malloc(sizeof(DirectoryElement) * MAX_FILE_SIZE);
    if(!fs->curr_directory.elements){
        handle_error("Error in allocating memory for directory elements");
        free(fs->buff);
        free(fs);
        return NULL;
    }

    fs->curr_directory.num_elements = 0;

    return fs;
}