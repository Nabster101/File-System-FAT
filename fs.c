#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

FileSystem* init_fs(int buff_size){

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if(!fs){
        handle_error("Error in allocating memory for file system\n");
    }

    fs->buff = (char*)malloc(buff_size);
    if(!fs->buff){
        handle_error("Error in allocating memory for buffer\n");
    }

    fs->buff_size = buff_size;
    fs->curr_directory.elements = (DirectoryElement*)malloc(sizeof(DirectoryElement) * MAX_FILE_SIZE);
    if(!fs->curr_directory.elements){
        handle_error("Error in allocating memory for directory elements\n");
        free(fs->buff);
        free(fs);
    }

    fs->curr_directory.num_elements = 0;

    printf("File System initialized successfully!\n");

    return fs;
}

void create_file(FileSystem *fs, char *name){
    
    if(!fs){                                                               // File system not found
        handle_error("File System not found!");
    }

    for(int i = 0; i < fs->curr_directory.num_elements; i++){              // Checking wether the name is already in use 
        if(strcmp(fs->curr_directory.elements[i].name, name) == 0){
            handle_error("File with this name already exists!\n");
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
        handle_error("Buffer is full!\n");
    }

    DirectoryElement new_element;
    strcpy(new_element.name, name);                                      // we "copy" the file name to the new_element DirectoryElement with its position in the buffer
    new_element.pos = flag;

    fs->curr_directory.elements = realloc(fs->curr_directory.elements, sizeof(DirectoryElement) * (fs->curr_directory.num_elements + 1));       // we realloc the list of elements in the current directory in order to add this new element
    if(!fs->curr_directory.elements){
        handle_error("Error in reallocating memory for directory elements\n");
    }

    fs->curr_directory.elements[fs->curr_directory.num_elements] = new_element;         // we add the new element to the list of elements in the current directory
    fs->curr_directory.num_elements++;                                                  // we increment the number of elements in the current directory

    printf("%s created successfully!\n", name);
    return;

}

void erase_file(FileSystem *fs, char *name){
    
    if (!fs){
        handle_error("File System not found!\n");
    }

    int file_found = -1;

    for(int i = 0; i < fs->curr_directory.num_elements; i++){
        if(strcmp(fs->curr_directory.elements[i].name, name) == 0){                         // we search for the file in the current directory  
        
            memset(fs->buff + fs->curr_directory.elements[i].pos, 0, MAX_FILE_SIZE);        // we erase the file in the buffer
            memmove(&(fs->curr_directory.elements[i]), &(fs->curr_directory.elements[i+1]), sizeof(DirectoryElement) * (fs->curr_directory.num_elements-i-1));  // we move the elements in the directory to the left

            fs->curr_directory.elements = realloc(fs->curr_directory.elements, sizeof(DirectoryElement) * (fs->curr_directory.num_elements - 1));               // we realloc the list of elements in the current directory in order to remove this element
            fs->curr_directory.num_elements--;                                              // we decrement the number of elements in the current directory :)

            printf("%s erased successfully!\n", name);
            file_found = 1;
            break;

        }
    }

    if(file_found == -1) printf("File not found!\n");

}

void write_file(FileSystem *fs, char *name, char *content){
    
    if(!fs){
        handle_error("File System not found!\n");
    }

    for(int i = 0; i < fs->curr_directory.num_elements; i++){
        if(strcmp(fs->curr_directory.elements[i].name, name) == 0){                     // we search for the file in the current directory

            int content_size = strlen(content);                                         // size of the content
            if(content_size > MAX_FILE_SIZE){                                           // we check if the content size is greater than the maximum file size
                handle_error("Content size is greater than the maximum file size\n");
            }
            
            if(fs->curr_directory.elements[i].pos + content_size > fs->buff_size){      // we check if the content size is greater than the buffer size
                handle_error("Content size is greater than the buffer size\n");
            }

            strcpy(fs->buff + fs->curr_directory.elements[i].pos, content);             // we copy the content to the buffer
            printf("Content written successfully to %s!\n", name);
            return;

        }else{
            handle_error("File not found!\n");
        }
    }

}