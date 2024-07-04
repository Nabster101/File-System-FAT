#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FileSystem* init_fs(int buff_size){

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if(!fs){
        handle_error_ret("\n#### ERROR in allocating memory for file system! ####\n", NULL);
    }

    fs->buff = (char*)malloc(buff_size);
    if(!fs->buff){
        free(fs);
        handle_error_ret("\n#### ERROR in allocating memory for buffer! ####\n", NULL);
    }

    fs->buff_size = buff_size;
    fs->root_directory = (Directory*)malloc(sizeof(Directory));
    if(!fs->root_directory){
        free(fs->buff);
        free(fs);
        handle_error_ret("\n#### ERROR in allocating memory for root directory! ####\n", NULL);
    }

    strncpy(fs->root_directory->dir_name, "root", MAX_FILE_NAME);
    fs->root_directory->elements = NULL;
    fs->root_directory->num_elements = 0;
    fs->root_directory->num_subdirectories = 0;
    fs->root_directory->parent = NULL;

    fs->curr_directory = fs->root_directory;

    printf("\n#### HOLD UP! File System initialized successfully! ####\n");

    return fs;
}

FileHandler* create_file(FileSystem *fs, const char *name){

    if (!fs){                                                       // File system not found
        handle_error_ret("\n#### ERROR! File System not found! ####\n", NULL);
    }

    for(int i = 0; i < fs->curr_directory->num_elements; i++){              // Checking wether the name is already in use 
        if(strcmp(fs->curr_directory->elements[i].name, name) == 0){
            handle_error_ret("\n #### ERROR! File with this name already exists! ####\n", NULL);
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
        perror("\n#### ERROR! Buffer is full! ####\n");
        return NULL;
    }

    DirectoryElement *new_elements = realloc(fs->curr_directory->elements, sizeof(DirectoryElement) * (fs->curr_directory->num_elements + 1));
    if (!new_elements){
        handle_error_ret("\n#### ERROR in reallocating memory for directory elements! ####\n", NULL);
    }
    fs->curr_directory->elements = new_elements;

    DirectoryElement* new_element = &fs->curr_directory->elements[fs->curr_directory->num_elements++];  // Creating a new DirectoryElemen
    strncpy(new_element->name, name, MAX_FILE_NAME);                                      // we "copy" the file name to the new_element DirectoryElement with its position in the buffer
    new_element->pos = flag;
    new_element->parent = fs->curr_directory;

    printf("\n#### HOLD UP! %s created successfully! ####\n", name);

    FileHandler* fh = (FileHandler*)malloc(sizeof(FileHandler));                        // we create a new FileHandler
    if(!fh){
        handle_error_ret("\n#### ERROR in allocating memory for file handler! ####\n", NULL);
    }

    fh->pos = 0;                                                                        // we set the position of the file handler to 0
    fh->element_index = fs->curr_directory->num_elements - 1;                             // we set the element index of the file handler to the index of the new element in the current directory
    fh->directory = fs->curr_directory;                                                // we set the directory of the file handler to the current directory

    return fh;

}

void erase_file(FileSystem *fs, const char *name){
    
    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    int file_found = -1;

    for(int i = 0; i < fs->curr_directory->num_elements; i++){
        if(strcmp(fs->curr_directory->elements[i].name, name) == 0){                         // we search for the file in the current directory  
        
            memset(fs->buff + fs->curr_directory->elements[i].pos, 0, MAX_FILE_SIZE);        // we erase the file in the buffer
            memmove(&(fs->curr_directory->elements[i]), &(fs->curr_directory->elements[i+1]), sizeof(DirectoryElement) * (fs->curr_directory->num_elements-i-1));  // we move the elements in the directory to the left

            DirectoryElement *new_elements = realloc(fs->curr_directory->elements, sizeof(DirectoryElement) * (fs->curr_directory->num_elements - 1));
            if (new_elements || fs->curr_directory->num_elements - 1 == 0){
                fs->curr_directory->elements = new_elements;
            }
            fs->curr_directory->num_elements--; // we decrement the number of elements in the current directory :)

            printf("\n#### HOLD UP! %s erased successfully! ####\n", name);
            file_found = 1;
            break;

        }
    }

    if(file_found == -1){
        printf("\n#### ERROR! %s not found! ####\n", name);
        return;
    }

}

void write_file(FileSystem *fs, FileHandler *fh, const char *data){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    if(!fh){
        handle_error("\n#### ERROR! File Handler not found! ####\n");
        return;
    }

    DirectoryElement *element = &fh->directory->elements[fh->element_index];

    int content_size = strlen(data);
    if(content_size > MAX_FILE_SIZE){
        handle_error("\n#### ERROR! Content size is greater than the maximum file size! ####\n");
        return;
    }

    if(element->pos + fh->pos + content_size > fs->buff_size){
        handle_error("\n#### ERROR! Content size is greater than the buffer size! ####\n");
        return;
    }

    memcpy(fs->buff + element->pos + fh->pos, data, content_size);
    printf("\n#### HOLD UP! Content written successfully to %s! ####\n", element->name);
    return;
}

void read_file(FileSystem *fs, FileHandler *fh){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    if (!fh){
        handle_error("\n#### ERROR! File Handler not found! ####\n");
        return;
    }

    DirectoryElement *element = &fh->directory->elements[fh->element_index];

    printf("\n#### Content of %s: %s ####\n", element->name, fs->buff + element->pos + fh->pos);

    return;
}

void seek_file(FileHandler *fh, int pos){

    if(pos < 0){
        handle_error("\n#### ERROR! Position is less than 0! ####\n");
    }

    if(!fh){
        handle_error("\n#### ERROR! File Handler not found! ####\n");
    }

    fh->pos = pos;                                          // we set the file cursor to the position we want

    return;

}

void create_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    for(int i = 0; i < fs->curr_directory->num_subdirectories; i++){
        if(strcmp(fs->curr_directory->subdirectories[i]->dir_name, name) == 0){
            handle_error("\n#### ERROR! Directory with this name already exists! ####\n");
            return;
        }
    }

    Directory *new_directory = (Directory*)malloc(sizeof(Directory));
    if(!new_directory){
        handle_error("\n#### Error in allocating memory for new directory! ####\n");
        return;
    }

    strncpy(new_directory->dir_name, name, MAX_FILE_NAME);
    new_directory->elements = NULL;
    new_directory->num_elements = 0;
    new_directory->num_subdirectories = 0;
    new_directory->parent = fs->curr_directory;

    fs->curr_directory->subdirectories[fs->curr_directory->num_subdirectories++] = new_directory;

    printf("\n#### HOLD UP! %s created successfully! ####\n", name);

    return;
}

void erase_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    int dir_found = -1;

    for (int i = 0; i < fs->curr_directory->num_subdirectories; i++){
        if (strcmp(fs->curr_directory->subdirectories[i]->dir_name, name) == 0){

            Directory *dir_to_erase = fs->curr_directory->subdirectories[i];
            while (dir_to_erase->num_subdirectories > 0){
                erase_directory(fs, dir_to_erase->subdirectories[dir_to_erase->num_subdirectories - 1]->dir_name);
                dir_to_erase->num_subdirectories--;
            }

            while (dir_to_erase->num_elements > 0){
                erase_file(fs, dir_to_erase->elements[dir_to_erase->num_elements - 1].name);
                dir_to_erase->num_elements--;
                if (dir_to_erase->num_elements == 0){
                    break;
                }
            }

            memmove(&(fs->curr_directory->subdirectories[i]), &(fs->curr_directory->subdirectories[i + 1]), sizeof(Directory *) * (fs->curr_directory->num_subdirectories - i - 1));
            fs->curr_directory->num_subdirectories--;

            free(dir_to_erase);
            printf("\n#### HOLD UP! Directory %s erased successfully! ####\n", name);
            return;
        }
    }

    if(dir_found == -1){
        handle_error("\n#### ERRROR! Directory not found! ####\n");
        return;
    }
}

void change_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    if(strcmp(name, "..") == 0){
        if(fs->curr_directory->parent){
            fs->curr_directory = fs->curr_directory->parent;
            printf("\n#### HOLD UP! Directory changed to %s! ####\n", fs->curr_directory->dir_name);
            return;
        }
        else{
            printf("\n#### HOLD UP! Already in root directory! ####\n");
            return;
        }
    }

    int dir_found = -1;

    for (int i = 0; i < fs->curr_directory->num_subdirectories; i++){
        if (strcmp(fs->curr_directory->subdirectories[i]->dir_name, name) == 0){
            printf("\n#### HOLD UP! Directory changed to %s! ####\n", name);
            fs->curr_directory = fs->curr_directory->subdirectories[i];
            dir_found = 1;
            break;
        }
    }

    if(dir_found == -1){
        handle_error("\n#### ERRROR! Directory not found! ####\n");
        return;
    }

}

void list_directory(FileSystem *fs){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    printf("\n------ Current Directory: %s ------- \n", fs->curr_directory->dir_name);

    printf("Directories:\n");
    for (int i = 0; i < fs->curr_directory->num_subdirectories; i++){
        printf("  %s\n", fs->curr_directory->subdirectories[i]->dir_name);
    }

    printf("Files:\n");
    for (int i = 0; i < fs->curr_directory->num_elements; i++){
        printf("  %s\n", fs->curr_directory->elements[i].name);
    }

    printf("--------------------------------------\n");
}