#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FileSystem* init_fs(int buff_size){

    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));                                               // we allocate memory for the file system
    if(!fs){
        handle_error_ret("\n#### ERROR in allocating memory for file system! ####\n", NULL);                // if there's an error we return NULL
    }

    fs->buff = (char*)malloc(buff_size);                                                                    // we allocate memory for the buffer
    if(!fs->buff){
        free(fs);
        handle_error_ret("\n#### ERROR in allocating memory for buffer! ####\n", NULL);                     // if there's an error we return NULL            
    }

    fs->buff_size = buff_size;                                                                              // we set the buffer size to the size we want                                             
    fs->root_directory = (Directory*)malloc(sizeof(Directory));                                             // we allocate memory for the root directory                        
    if(!fs->root_directory){                                                                                // if there's an error we free the buffer and the file system and return NULL
        free(fs->buff);
        free(fs);
        handle_error_ret("\n#### ERROR in allocating memory for root directory! ####\n", NULL);
    }

    strncpy(fs->root_directory->dir_name, "root", MAX_FILE_NAME);                                           // we set the name of the root directory to "root"
    fs->root_directory->elements = NULL;                                                                    // we set the elements of the root directory to NULL
    fs->root_directory->num_elements = 0;                                                                   // we set the number of elements in the root directory to 0 
    fs->root_directory->num_subdirectories = 0;                                                             // we set the number of subdirectories in the root directory to 0
    fs->root_directory->parent = NULL;                                                                      // we set the parent of the root directory to NULL (it's the root)

    fs->curr_directory = fs->root_directory;                                                                // we set the current directory to the root directory                                  

    printf("\n#### HOLD UP! File System initialized successfully! ####\n");

    return fs;                                                                                              // we return the file system                                          
}

FileHandler* create_file(FileSystem *fs, const char *name){

    if (!fs){                                                                                               // File system not found
        handle_error_ret("\n#### ERROR! File System not found! ####\n", NULL);
    }

    for(int i = 0; i < fs->curr_directory->num_elements; i++){                                              // Checking whether the name is already in use 
        if(strcmp(fs->curr_directory->elements[i].name, name) == 0){                                        // it goes through all the elements in the current directory and checks the name
            handle_error_ret("\n #### ERROR! File with this name already exists! ####\n", NULL);
        }
    }

    int flag = -1;
    for(int i = 0; i < fs->buff_size; i++){                                                                 // Checking if there's some free space in the File System buffer
        if(fs->buff[i] == '\0'){                                                                            // If there is space, the block will recieve position of flag in the FS buffer (first free block) :)
            flag = i;
            break;                              
        }
    }

    if(flag == -1){                                                                                         // If there isn't any space the buffer is full :(
        perror("\n#### ERROR! Buffer is full! ####\n");
        return NULL;
    }

    DirectoryElement *new_elements = realloc(fs->curr_directory->elements, sizeof(DirectoryElement) * (fs->curr_directory->num_elements + 1));  // we reallocate memory in the current directory to be able to add a new element
    if (!new_elements){
        handle_error_ret("\n#### ERROR in reallocating memory for directory elements! ####\n", NULL);
    }
    fs->curr_directory->elements = new_elements;    

    DirectoryElement* new_element = &fs->curr_directory->elements[fs->curr_directory->num_elements++];     // Creating a new DirectoryElement thas has the address of the last element in the current directory
    strncpy(new_element->name, name, MAX_FILE_NAME);                                                       // we "copy" the file name to the new_element DirectoryElement with its position in the buffer
    new_element->pos = flag;
    new_element->parent = fs->curr_directory;                                                              // we set the parent of the new element to the current directory    

    printf("\n#### HOLD UP! %s created successfully! ####\n", name);

    FileHandler* fh = (FileHandler*)malloc(sizeof(FileHandler));                                           // we create a new FileHandler
    if(!fh){
        handle_error_ret("\n#### ERROR in allocating memory for file handler! ####\n", NULL);
        free(new_elements);
    }

    fh->pos = 0;                                                                                           // we set the position of the file handler to 0
    fh->element_index = fs->curr_directory->num_elements - 1;                                              // we set the element index of the file handler to the index of the new element in the current directory
    fh->directory = fs->curr_directory;                                                                    // we set the directory of the file handler to the current directory

    return fh;

}

void erase_file(FileSystem *fs, const char *name){
    
    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    int file_found = -1;                                                                                  // we set the file_found flag to -1

    for(int i = 0; i < fs->curr_directory->num_elements; i++){
        if(strcmp(fs->curr_directory->elements[i].name, name) == 0){                                      // we search for the file in the current directory  
        
            memset(fs->buff + fs->curr_directory->elements[i].pos, 0, MAX_FILE_SIZE);                     // we erase the file in the buffer (buffer + position of the file in the buffer)
            memmove(&(fs->curr_directory->elements[i]), &(fs->curr_directory->elements[i+1]), sizeof(DirectoryElement) * (fs->curr_directory->num_elements-i-1));  // we move the elements in the directory to the left

            DirectoryElement *new_elements = realloc(fs->curr_directory->elements, sizeof(DirectoryElement) * (fs->curr_directory->num_elements - 1)); // we reallocate memory in the current directory to be able to remove the element
            if (new_elements || fs->curr_directory->num_elements - 1 == 0){                              // if there's no error or the number of elements in the current directory is 0                   
                fs->curr_directory->elements = new_elements;                                             // we set the new elements in the current directory
            }

            if(fs->curr_directory->num_elements > 0) fs->curr_directory->num_elements--;                 // we decrement the number of elements in the current directory :)

            printf("\n#### HOLD UP! %s erased successfully! ####\n", name);
            file_found = 1;                                                                              // file has been found! :)
            break;

        }
    }

    if(file_found == -1){                                                                               // if the file has not been found :(                                       
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

    DirectoryElement *element = &fh->directory->elements[fh->element_index];                            // we get the file from the directory based off its index in the directory

    int content_size = strlen(data);
    if(content_size > MAX_FILE_SIZE){
        handle_error("\n#### ERROR! Content size is greater than the maximum file size! ####\n");
        return;
    }

    if(element->pos + fh->pos + content_size > fs->buff_size){                                          // if the position of the file in the buffer + the position of the file cursor + the content size is greater than the buffer size
        handle_error("\n#### ERROR! Content size is greater than the buffer size! ####\n");
        return;
    }

    memcpy(fs->buff + element->pos + fh->pos, data, content_size);                                      // we copy the data to the buffer at the position of the file in the buffer + the position of the file cursor
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

    printf("\n#### Content of %s: %s ####\n", element->name, fs->buff + element->pos + fh->pos);        // we print the content of the file in the buffer at the position of the file in the buffer + the position of the file cursor

    return;
}

void seek_file(FileHandler *fh, int pos){

    if(pos < 0){
        handle_error("\n#### ERROR! Position is less than 0! ####\n");
    }

    if(!fh){
        handle_error("\n#### ERROR! File Handler not found! ####\n");
    }

    fh->pos = pos;                                                                                     // we set the file cursor to the position we want

    return;

}

void create_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    for(int i = 0; i < fs->curr_directory->num_subdirectories; i++){                                  // Checking whether the name is already in use  
        if(strcmp(fs->curr_directory->subdirectories[i]->dir_name, name) == 0){
            handle_error("\n#### ERROR! Directory with this name already exists! ####\n");
            return;
        }
    }

    Directory *new_directory = (Directory*)malloc(sizeof(Directory));                                // if not we allocate memory for the new directory
    if(!new_directory){
        handle_error("\n#### Error in allocating memory for new directory! ####\n");
        return;
    }

    strncpy(new_directory->dir_name, name, MAX_FILE_NAME);                                          // we set directory parameters
    new_directory->elements = NULL;
    new_directory->num_elements = 0;
    new_directory->num_subdirectories = 0;
    new_directory->parent = fs->curr_directory;                                                     // we set the parent of the new subdirectory to the "current directory" on which the File System is working on

    fs->curr_directory->subdirectories[fs->curr_directory->num_subdirectories++] = new_directory;   // we add the new directory to the subdirectories of the current directory

    printf("\n#### HOLD UP! %s created successfully! ####\n", name);

    return;
}

void erase_directory(FileSystem *fs, const char *name){

    if (!fs){
        handle_error("\n#### ERROR! File System not found! ####\n");
        return;
    }

    int dir_found = -1;                                                                             // we set the dir_found flag to -1

    for (int i = 0; i < fs->curr_directory->num_subdirectories; i++){                               // we search for the subdirectory in the current directory                   
        if (strcmp(fs->curr_directory->subdirectories[i]->dir_name, name) == 0){

            Directory *dir_to_erase = fs->curr_directory->subdirectories[i];
            while (dir_to_erase->num_subdirectories > 0){
                erase_directory(fs, dir_to_erase->subdirectories[dir_to_erase->num_subdirectories - 1]->dir_name);      // we delete all the subdirectories of the directory we want to erase
                dir_to_erase->num_subdirectories--;
            }

            while (dir_to_erase->num_elements > 0){                                                                     // we delete all the files in the directory we want to erase
                erase_file(fs, dir_to_erase->elements[dir_to_erase->num_elements - 1].name);
                dir_to_erase->num_elements--;
                if (dir_to_erase->num_elements == 0){
                    break;
                }
            }

            memmove(&(fs->curr_directory->subdirectories[i]), &(fs->curr_directory->subdirectories[i+1]), sizeof(Directory*) * (fs->curr_directory->num_subdirectories-i-1));  // we move the subdirectories to the left
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

    if(strcmp(name, "..") == 0){                                                                                        // if the name is ".." we go to the parent directory                                       
        if(fs->curr_directory->parent){                                                                                 // if the parent of the current directory is not NULL (root)
            fs->curr_directory = fs->curr_directory->parent;                                                            // we set the current directory to the parent directory
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
            fs->curr_directory = fs->curr_directory->subdirectories[i];                                                 // we set the current directory to the subdirectory we want to go to
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

void handle_create_file(FileSystem *fs, const char *name){
    FileHandler *fh = create_file(fs, name);                                            // we create a file handler and call the create_file function
}

void handle_erase_file(FileSystem *fs, const char *name){
    erase_file(fs, name);                                                               // we call the erase_file function
}

void handle_write_file(FileSystem *fs, const char *name, const char *data){
    FileHandler fh;

    fh.directory = fs->curr_directory;                                                  // we set the directory of the file handler to the current directory
    for(int i = 0; i < fh.directory->num_elements; i++){
        if(strcmp(fh.directory->elements[i].name, name) == 0){
            fh.element_index = i;                                                       // we set the element index of the file handler to the index of the file in the directory
            write_file(fs, &fh, data);                                                  // we call the write_file function passing the file system, the file handler and the data
            return;
        }
    }
    printf("\n#### ERROR! File not found! ####\n");
}

void handle_read_file(FileSystem *fs, const char *name){

    FileHandler fh;
    fh.directory = fs->curr_directory;
    for(int i = 0; i < fh.directory->num_elements; i++){
        if(strcmp(fh.directory->elements[i].name, name) == 0){
            fh.element_index = i;
            read_file(fs, &fh);
            return;
        }
    }
    printf("\n#### ERROR! File not found! ####\n");
}

void handle_create_directory(FileSystem *fs, const char *name){
    create_directory(fs, name);
}

void handle_change_directory(FileSystem *fs, const char *name){
    change_directory(fs, name);
}

void handle_list_directory(FileSystem *fs){
    list_directory(fs);
}

void handle_erase_directory(FileSystem *fs, const char *name){
    erase_directory(fs, name);
}

void free_fs(FileSystem *fs){

    if (!fs) return;

    if (fs->buff) free(fs->buff);

    free(fs->root_directory);

    free(fs);
}