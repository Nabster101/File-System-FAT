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

void create_file(FileSystem *fs, char *name){
    
    if(!fs){                                                               // File system not found
        handle_error("File System not found!");
    }

    for(int i = 0; i < fs->curr_directory.num_elements; i++){              // Checking wether the name is already in use 
        if(strcmp(fs->curr_directory.elements[i].name, name) == 0){
            handle_error("File with this name already exists!");
            exit(EXIT_FAILURE);
        }
    }

    int flag = -1;
    for(int i = 0; i < fs->buff_size; i++){                               // Checking if there's some free space in the File System buffer
        if(fs->buff[i] == '\0'){                                          // If there is space, the block will recieve position of flag in the FS buffer :)
            flag = i;
            break;                              
        }
    }

    if(flag == -1){                                                       // If there isn't any space the buffer is full :(
        handle_error("Buffer is full!");
        exit(EXIT_FAILURE);
    }

    DirectoryElement new_element;
    strcpy(new_element.name, name);                                      // we "copy" the file name to the new_element DirectoryElement with its position in the buffer
    new_element.pos = flag;

    fs->curr_directory.elements = realloc(fs->curr_directory.elements, sizeof(DirectoryElement) * (fs->curr_directory.num_elements + 1));       // we realloc the list of elements in the current directory in order to add this new element
    if(!fs->curr_directory.elements){
        handle_error("Error in reallocating memory for directory elements");
        exit(EXIT_FAILURE);
    }

    fs->curr_directory.elements[fs->curr_directory.num_elements] = new_element;
    fs->curr_directory.num_elements++;

}