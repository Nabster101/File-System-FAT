#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int *fat = NULL;                                                                                // File Allocation Table poiner 
DirectoryElement *root = NULL;                                                                  // Root directory pointer
DirectoryElement *current_directory = NULL;                                                     // Current directory pointer
DirectoryElement *parent_directory = NULL;                                                      // Parent directory pointer
void *fs_start = NULL;                                                                          // File system start pointer
int root_size = 0;                                                                              // initial Root size                                          
int fat_size = 0;                                                                               // initial FAT size 

int init_fs(const char* fileImage, int size){
    
    if(!fileImage){                                                                             // Checking whether the file image is found                                                 
        handle_error_ret("\n#### ERROR! File image not found! ####\n", -1);
    }

    if(size <= 0){                                                                              // Checking whether the size is greater than 0
        handle_error_ret("\n#### ERROR! Size is less than or equal to 0! ####\n", -1);
    }

    int fd = open(fileImage, O_RDWR | O_CREAT, 0666);                                           // Opening the file image in read-write mode                           
    if(fd == -1){                                                                                           
        handle_error_ret("\n#### ERROR! Couldn't open the fileImage! ####\n", -1);
    }

    if(ftruncate(fd, size) == -1){                                                              // Truncating the file image to the specified size
        handle_error_ret("\n#### ERROR! Couldn't truncate the fileImage! ####\n", -1);
    }         

    fs_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);                     // Mapping the file image to the memory and setting the starting address of the file system
    if(fs_start == MAP_FAILED){                                                                            
        handle_error_ret("\n#### ERROR! Couldn't map the fileImage! ####\n", -1);
    }

    fat = fs_start;                                                                             // we set the start of the FAT at the start of the File System
    root = (DirectoryElement*) (fs_start + CLUSTER_SIZE);                                       // we set the start of the root directory at the start of the File System + the size of the FAT

    // 0 means that the block is free
    // -1 means that the block is reserved
    // -2 means that the block is the last block in the file (EOF)

    memset(fat, 0, FAT_ELEMENTS * 4);                                                           // we set the FAT to 0 (we multiply the number of elements in the FAT by 4 because in FAT32 each element is 4 bytes long)                     
    printf("FAT initialized!\n");
    
    memset(root, 0, (size - CLUSTER_SIZE));                                                     // we set the root directory to 0 (we subtract the size of the FAT from the size of the File System because the root directory starts after the FAT)
    printf("Root initialized!\n");

    root_size = sizeof(DirectoryElement);                                                       // we set the root size to the size of the DirectoryElement structure
    fat_size = FAT_ELEMENTS;                                                                    // we set the FAT size to the number of elements in the FAT

    current_directory = root;                                                                   // we set the current directory to the root directory
    current_directory->size = root_size;
    current_directory->is_directory = 1;                                                        // we set the current directory to a directory
    current_directory->start_block = 0;
    fat[current_directory->start_block] = -1;                                                   // we set the start block of the current directory in the FAT to -1
    strncpy(current_directory->name, "ROOT", MAX_FILE_NAME);                                    // we set the name of the current directory to ROOT

    parent_directory = current_directory;                                                                    // we set the parent directory to the root directory
    return 0;                                                                                                                                   
}

int free_fat_block(){                                                                           // Function to get a free block in the FAT
    
    for(int i = 2; i < fat_size; i++){                                                          // we start from 2 because the first two blocks are reserved to the current directory
        if(fat[i] == 0){
            return i;
        }
    }
    
    return -1;

}

FileHandler* create_file(const char *name){
    
        if(!name){
            handle_error_ret("\n#### ERROR! File name not found! ####\n", NULL);
        }
    
        if(strlen(name) > MAX_FILE_NAME){
            handle_error_ret("\n#### ERROR! File name is too long! ####\n", NULL);
        }

        for(int i = 0; i < current_directory->size; i++){                                                                                // Checking whether the name is already in use in the current directory
            if(strcmp(current_directory[i].name, name) == 0){
                handle_error_ret("\n#### ERROR! File with this name already exists! ####\n", NULL);
            }
        }
    
        int free_block = free_fat_block();                                                                                               // Getting a free block in the FAT                           
        if(free_block == -1){
            handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", NULL);
        }

    
        for(int i = 0; i < current_directory->size; i++){                                                                                // Searching for a free block in the current directory
            if(current_directory[i].size == 0 && current_directory[i].name[0] == '\0' && current_directory[i].is_directory == 0){
                strncpy(current_directory[i].name, name, MAX_FILE_NAME-2);                                                               // Copying the name to the current directory
                current_directory[i].name[MAX_FILE_NAME-1] = '\0';
                current_directory[i].start_block = free_block;
                current_directory[i].parent = current_directory;
                fat[free_block] = -2;
                current_directory->size = sizeof(DirectoryElement)*i;
                current_directory[i].is_directory = 0;
                break;
            }
        }

    
        FileHandler *fh = (FileHandler*)malloc(sizeof(FileHandler));                                                                     // Allocating memory for the file handler
        if(!fh){
            handle_error("\n#### ERROR! Couldn't allocate memory for the file handler! ####\n");
            return NULL;
        }
    
        fh->pos = 0;                                                                                                                    // Setting the current position in the file to 0
        fh->directory = current_directory;                                                                                              // Setting the directory of the file handler to the current directory
        strncpy(fh->file_name, name, MAX_FILE_NAME-1);                                                                                  // Copying the name to the file handler
        fh->file_name[MAX_FILE_NAME-1] = '\0';
    
        printf("\n#### File %s created successfully! ####\n", name);
    
        return fh;
}

int erase_file(const char *name){
    
    if(!name){
        handle_error_ret("\n#### ERROR! File name not found! ####\n", -1);
    }

    for(int i = 1; i < current_directory->size; i++){                                       // Searching for the file in the current directory
        if(strcmp(current_directory[i].name, name) == 0){                                   // If the file is found

            int block = current_directory[i].start_block;  
            printf("CUR DIR SIZE: %s\n", current_directory->name);
            printf("BLOCK: %d\n", block);                                                         // Getting the start block of the file in the FAT
            printf("Erasing file: %s\n", name);
            while(fat[block] != 0){
                if(fat[block] == -2){
                    fat[block] = 0;
                    printf("Freed block: %u\n", block);
                    break;
                }
                int next_block = fat[block];
                fat[block] = 0;
                printf("Freed block: %u\n", block);
                block = next_block;
            }

            memset(&current_directory[i], 0, sizeof(DirectoryElement));
            printf("File %s erased successfully!\n", name);
            return 0;

        }
    }

    return -1;
}


int write_file(FileHandler *fh, const char *data){

    if(!fh){
        handle_error_ret("\n#### ERROR! File handler not found! ####\n", -1);
    }

    if(!data){
        handle_error_ret("\n#### ERROR! Data not found! ####\n", -1);
    }

    int data_size = strlen(data);                                                            // Getting the size of the data
    int data_pos = fh->pos;                                                                  // Getting the current position in the file
    int rem_size = data_size;                                                                // Getting the remaining size of the data
    int bytes_written = 0;                                                                   // Number of bytes written to the file
    DirectoryElement *file = NULL;

    int is_in_current_directory = 0;


    if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){
        is_in_current_directory = 1;
    }

    if(is_in_current_directory){
        for(int i = 0; i < current_directory->size; i++){                                                   // Searching for the file in the current directory
            if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){
                file = &current_directory[i];
                file->current_block = file->start_block;
                break;
            }
        }
    }else{
        handle_error_ret("\n#### ERROR! File not found in the current directory! ####\n", -1);
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }


    int block = file->start_block;                                                         // Getting the start block of the file in the FAT      
    printf("BLOCK: %d | FILE: %s\n", block, fh->file_name);


                                                                                   // with this loop, we are traversing the FAT to the block where the fh->pos is pointing to

    while(rem_size > 0){

        if(fat[block] == -2){                                                       // empty file
            int new_block = free_fat_block();
            if(new_block == -1){
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            fat[block] = new_block;
            file->current_block = block;
            fat[new_block] = -2;
        }

        char *block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block));                     // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
        int write_size = CLUSTER_SIZE - data_pos;                                       // Getting the write size (number of bytes to write to the block) by subtracting the byte offset from the cluster size
        if(write_size > rem_size){                                                         // If the write size is greater than the remaining size of the data, then we set the write size to the remaining size of the data
            write_size = rem_size;
        }

        memcpy(block_data + data_pos, data + bytes_written, write_size);                // Copying the data to the block data starting from the byte offset
        bytes_written += write_size;                                                       // Updating the number of bytes written to the file                                  
        rem_size -= write_size;                                                            // Updating the remaining size of the data
        data_pos = 0;                                                                     // Resetting the byte offset to 0 (since we are writing to a new block)


        fh->pos += write_size;                                                             // Updating the current position in the file
    }

    file->size += bytes_written;
    return bytes_written;
}


int read_file(FileHandler *fh, char*buff, int buff_size){

    if(!fh){
        handle_error_ret("\n#### ERROR! File handler not found! ####\n", -1);
    }

    if(!buff){
        handle_error_ret("\n#### ERROR! Buffer not found! ####\n", -1);
    }

    int data_pos = fh->pos;                                                               // Getting the current position in the file
    int rem_size = buff_size;                                                             // Getting the remaining size of the buffer
    int bytes_read = 0;                                                                   // Number of bytes read from the file

    DirectoryElement *file = NULL;

    int is_in_current_directory = 0;

    
    if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){
        is_in_current_directory = 1;
    }

    if(is_in_current_directory){
        for(int i = 0; i < current_directory->size; i++){                                                   // Searching for the file in the current directory
            if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){
                file = &current_directory[i];
                break;
            }
        }
    }else{
        handle_error_ret("\n#### ERROR! File not found in the current directory! ####\n", -1);
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int block = file->start_block;                                                        // Getting the start block of the file in the FAT
                                                                                 // with this loop, we are moving through the FAT to the block where the fh->pos is pointing to


    while(rem_size > 0 && fat[block] != -2){
        char *block_data = (char*)(fs_start + CLUSTER_SIZE * (block));                    // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
        int read_size = CLUSTER_SIZE - data_pos;                                       // Getting the read size (number of bytes to read from the block) by subtracting the byte offset from the cluster size
        if(read_size > rem_size){                                                         // If the read size is greater than the remaining size of the buffer, then we set the read size to the remaining size of the buffer
            read_size = rem_size;
        }

        memcpy(buff + bytes_read, block_data + data_pos, read_size);                  // Copying the data from the block data starting from the byte offset to the buffer
        bytes_read += read_size;                                                         // Updating the number of bytes read from the file
        rem_size -= read_size;                                                           // Updating the remaining size of the buffer
        data_pos = 0;                                                                 // Resetting the byte offset to 0 (since we are reading from a new block)
        fh->pos += read_size;                                                            // Updating the current position in the file

        if(rem_size > 0){                                                                // If the remaining size of the buffer is greater than 0, then we move to the next block in the FAT
            block = fat[block];
        }
    }

    if (bytes_read < buff_size){                                                        // If the number of bytes read from the file is less than the buffer size, then we add a null terminator to the buffer
        buff[bytes_read] = '\0';
    }

    return bytes_read;
}

int seek_file(FileHandler *fh, int pos){
    
    if(!fh){
        handle_error_ret("\n#### ERROR! File handler not found! ####\n", -1);
    }

    if(pos < 0){
        handle_error_ret("\n#### ERROR! Position is less than 0! ####\n", -1);
    }
    DirectoryElement *file = NULL;

    for(int i = 0; i < current_directory->size; i++){                                                   // Searching for the file in the current directory
        if(strncmp(current_directory[i].name, fh->directory->name, MAX_FILE_NAME) == 0){
            file = &current_directory[i];
            break;
        }
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    if(pos > file->size){
        handle_error_ret("\n#### ERROR! Position is greater than the file size! ####\n", -1);
    }

    fh->pos = pos;                                                                       // Updating the current position in the file                   
    return 0;
}


int list_directory(){
    
    printf("\n#### LISTING DIRECTORY %s ####\n", current_directory->name);
    for(int i = 1; i < current_directory->size; i++){
        if(current_directory[i].name[0] != '\0'){
            if(current_directory[i].is_directory){
                printf("Directory: %s\n", current_directory[i].name);
            }else{
                printf("File: %s\n", current_directory[i].name);
            }
        }
    }
    printf("\n");
    
    return 0;
}

int create_directory(const char *name){

    if (!name){
        handle_error_ret("\n#### ERROR! Directory name not found! ####\n", -1);
    }

    if (strlen(name) > MAX_FILE_NAME){
        handle_error_ret("\n#### ERROR! Directory name is too long! ####\n", -1);
    }

    for (int i = 0; i < current_directory->size; i++){
        if (strcmp(current_directory[i].name, name) == 0){
            handle_error_ret("\n#### ERROR! Directory with this name already exists! ####\n", -1);
        }
    }

    int free_block = free_fat_block();
    if (free_block == -1){
        handle_error("\n#### ERROR! No free blocks in the FAT! ####\n");
        return -1;
    }

    int found_empty_slot = 0;
    for (int i = 0; i < current_directory->size; i++){
        if (current_directory[i].size == 0 && current_directory[i].name[0] == '\0'){
            strncpy(current_directory[i].name, name, MAX_FILE_NAME - 1);
            current_directory[i].name[MAX_FILE_NAME - 1] = '\0';
            current_directory[i].start_block = free_block;
            printf("DIREC BLOCK: %d\n", free_block);
            printf("DIR NAME: %s\n", current_directory[i].name);
            current_directory[i].is_directory = 1;
            fat[free_block] = -1; 
            current_directory[i].size = sizeof(DirectoryElement);
            current_directory[i].current_block = free_block;
            found_empty_slot = 1;
            break;
        }
    }

    if (!found_empty_slot){
        handle_error("\n#### ERROR! No empty slot for new directory! ####\n");
        return -1;
    }

    current_directory->size += sizeof(DirectoryElement);

    printf("\n#### Directory %s created successfully! ####\n", name);

    return 0;
}



int change_directory(const char *name) {

    if (!name) {
        handle_error_ret("\n#### ERROR! Directory name not found! ####\n", -1);
    }

    if (strcmp(name, "..") == 0) {
        if (current_directory == parent_directory) {
            handle_error_ret("\n#### ERROR! Already in the root directory! ####\n", -1);
        }

        current_directory = parent_directory;
        printf("\n#### Changed directory to parent successfully! ####\n");
        return 0;
    }



    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory) {
            int block = current_directory[i].start_block;
            printf("BLOCK CHNAGE START PRE: %d\n", block);
            current_directory = (DirectoryElement *)(fs_start + CLUSTER_SIZE * block);
            strncpy(current_directory->name, name, MAX_FILE_NAME - 1);
            current_directory->size = sizeof(DirectoryElement);
            current_directory->is_directory = 1;
            printf("\n#### Changed directory to %s successfully! ####\n", name);
            return 0;
        }
    }

    handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
}

int erase_directory(const char *name) {
    
    if (!name) {
        handle_error_ret("\n#### ERROR! Directory name not found! ####\n", -1);
    }

    if(strcmp(name, current_directory->name) == 0){
        handle_error_ret("\n#### ERROR! Cannot erase the current directory! ####\n", -1);
    }

    int parent_block = 0;

    for(int i = 0; i < root_size; i++){
        for(int j = 0; j < current_directory->size; j++){
            if(strcmp(current_directory[j].name, root[i].name) == 0 && current_directory[j].is_directory == 1){
                parent_block = root[i].start_block;
            }
        }
            
    }


    printf("PARENT BLOCK: %d\n", parent_block);

    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory) {
            int block = current_directory[i].start_block;
            current_directory = (DirectoryElement *)(fs_start + CLUSTER_SIZE * block);


            for (int j = current_directory->size; j > 0; j--) {
                char *name = current_directory[j].name;
                if (current_directory[j].name[0] != '\0') {
                    if (current_directory[j].is_directory != 1) {
                        erase_file(name);
                    }
                }
            }
            printf("Directory %s files erased successfully!\n", name);
        }
    }

    current_directory = (DirectoryElement *)(fs_start + CLUSTER_SIZE * parent_block);

    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory) {
            int block = current_directory[i].start_block;
            DirectoryElement *dir = (DirectoryElement *)(fs_start + CLUSTER_SIZE * block);

            for (int j = 0; j < dir->size; j++) {
                if (dir[j].name[0] != '\0') {
                    if (dir[j].is_directory == 1) {
                        strncpy(dir[j].name, "\0", MAX_FILE_NAME);
                        dir[j].size = 0;
                        dir[j].is_directory = 0;
                        dir[j].start_block = 0;
                    }
                }
            }

            fat[block] = 0;
            memset(&current_directory[i], 0, sizeof(DirectoryElement));
            printf("Directory %s erased successfully!\n", name);
            return 0;
        }
    }


    handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
}