#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int *fat = NULL;
DirectoryElement *root = NULL;
void *fs_start = NULL;
int root_size = 0;
int fat_size = 0;

int init_fs(const char* fileImage, int size){
    
    if(!fileImage){                                                                                       
        handle_error_ret("\n#### ERROR! File image not found! ####\n", -1);
    }

    if(size <= 0){                
        handle_error_ret("\n#### ERROR! Size is less than or equal to 0! ####\n", -1);
    }

    int fd = open(fileImage, O_RDWR | O_CREAT, 0666); 
    if(fd == -1){                                                                                           
        handle_error_ret("\n#### ERROR! Couldn't open the fileImage! ####\n", -1);
    }

    if(ftruncate(fd, size) == -1){
        handle_error_ret("\n#### ERROR! Couldn't truncate the fileImage! ####\n", -1);
    }         

    fs_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if(fs_start == MAP_FAILED){                                                                            
        handle_error_ret("\n#### ERROR! Couldn't map the fileImage! ####\n", -1);
    }

    fat = fs_start;
    root = (DirectoryElement*) (fs_start + CLUSTER_SIZE);

    // 0 means that the block is the last block in the file (EOF)
    // -1 means that the block is free
    // -2 means that the block is reserved

    memset(fat, -1, FAT_ELEMENTS * 4);   
    printf("FAT initialized!\n");
    
    memset(root, 0, ((size - CLUSTER_SIZE) / sizeof(DirectoryElement) * sizeof(DirectoryElement)));
    printf("Root initialized!\n");

    root_size = (size - CLUSTER_SIZE) / sizeof(DirectoryElement);
    fat_size = FAT_ELEMENTS;
    
    return 0;                                                                                                                                   
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