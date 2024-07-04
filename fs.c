#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FileSystem* init_fs(int buff_size){

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if(!fs){
        handle_error("Error in allocating memory for file system\n");
        return NULL;
    }

    fs->buff = (char*)malloc(buff_size);
    if(!fs->buff){
        handle_error("Error in allocating memory for buffer\n");
        return NULL;
    }

    fs->buff_size = buff_size;
    fs->curr_directory.elements = (DirectoryElement*)malloc(sizeof(DirectoryElement) * MAX_FILE_SIZE);
    if(!fs->curr_directory.elements){
        handle_error("Error in allocating memory for directory elements\n");
        free(fs->buff);
        free(fs);
        return NULL;
    }

    fs->curr_directory.num_elements = 0;

    printf("File System initialized successfully!\n");

    return fs;
}

FileHandler* create_file(FileSystem *fs, const char *name){

    if (!fs){                                                       // File system not found
        perror("File System not found!");
        return NULL;
    }

    for(int i = 0; i < fs->curr_directory.num_elements; i++){              // Checking wether the name is already in use 
        if(strcmp(fs->curr_directory.elements[i].name, name) == 0){
            perror("File with this name already exists!\n");
            return NULL;
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
        perror("Buffer is full!\n");
        return NULL;
    }

    fs->curr_directory.elements = realloc(fs->curr_directory.elements, sizeof(DirectoryElement) * (fs->curr_directory.num_elements + 1)); // we realloc the list of elements in the current directory in order to add this new element
    if (!fs->curr_directory.elements){
        perror("Error in reallocating memory for directory elements\n");
        return NULL;
    }

    DirectoryElement* new_element = &fs->curr_directory.elements[fs->curr_directory.num_elements++];  // Creating a new DirectoryElemen
    strncpy(new_element->name, name, MAX_FILE_NAME);                                      // we "copy" the file name to the new_element DirectoryElement with its position in the buffer
    new_element->pos = flag;

    fs->curr_directory.elements[fs->curr_directory.num_elements] = *new_element;         // we add the new element to the list of elements in the current directory
    fs->curr_directory.num_elements++;                                                  // we increment the number of elements in the current directory

    printf("%s created successfully!\n", name);

    FileHandler* fh = (FileHandler*)malloc(sizeof(FileHandler));                        // we create a new FileHandler
    if(!fh){
        perror("Error in allocating memory for file handler\n");
        return NULL;
    }

    fh->pos = 0;                                                                        // we set the position of the file handler to 0
    fh->element_index = fs->curr_directory.num_elements - 1;                             // we set the element index of the file handler to the index of the new element in the current directory

    return fh;

}

void erase_file(FileSystem *fs, const char *name){
    
    if (!fs){
        handle_error("File System not found!\n");
        return;
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

    if(file_found == -1){
        printf("%s not found!\n", name);
        return;
    }

}

void write_file(FileSystem *fs, FileHandler *fh, const char *data){

    if (!fs){
        handle_error("File System not found!\n");
        return;
    }

    if(!fh){
        handle_error("File Handler not found!\n");
        return;
    }

    int file_found = -1;

    for(int i = 0; i < fs->curr_directory.num_elements; i++){
        if(strcmp(fs->curr_directory.elements[i].name, fs->curr_directory.elements[fh->element_index].name) == 0){      // we search for the file in the current directory with the file handler element index

            int content_size = strlen(data);                                                                            // size of the content
            if(content_size > MAX_FILE_SIZE){                                                                           // we check if the content size is greater than the maximum file size
                handle_error("Content size is greater than the maximum file size\n");
            }
            
            if(fs->curr_directory.elements[i].pos + fh->pos + content_size > fs->buff_size){                            // we check if the content size is greater than the buffer size
                handle_error("Content size is greater than the buffer size\n");
            }

            memcpy(fs->buff + fs->curr_directory.elements[i].pos, data, content_size);                                  // we copy the content to the buffer
            printf("Content written successfully to %s!\n", fs->curr_directory.elements[i].name);
            file_found = 1;
            return;

        }
    }

    if (file_found == -1) {
        printf("File not found!");
        return;
    }
}

void read_file(FileSystem *fs, FileHandler *fh){
    
    if(!fs){
        handle_error("File System not found!\n");
        return;
    }

    if(!fh){
        handle_error("File Handler not found!\n");
        return;
    }

    int file_found = -1;

    for(int i = 0; i < fs->curr_directory.num_elements; i++){
        if(strcmp(fs->curr_directory.elements[i].name, fs->curr_directory.elements[fh->element_index].name) == 0){                        // we search for the file in the current directory

            printf("Content of %s: %s\n", fs->curr_directory.elements[i].name, fs->buff + fs->curr_directory.elements[i].pos + fh->pos);  // we print the content of the file based on (buff pointer + absolute position in the buffer + relative position of the file cursor)
            file_found = 1;
            break;

        }
    }

    if(file_found == -1){
        printf("File not found!\n");
    }

}

void seek_file(FileHandler *fh, int pos){

    if(pos < 0){
        handle_error("Position is less than 0\n");
    }

    if(!fh){
        handle_error("File Handler not found!\n");
    }

    fh->pos = pos;                                          // we set the file cursor to the position we want

}

void create_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("File System not found!\n");
        return;
    }
}
