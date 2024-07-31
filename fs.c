#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/stat.h>
#include <sys/stat.h>

int *fat;                                                                                // File Allocation Table poiner 
char *data_blocks;                                                                       // Data blocks pointer
DirectoryElement *current_directory;                                                     // Current directory pointer
FileSystem *fs;                                                                                 // File system pointer
FILE *file_fs;                                                                                     // File pointer

int init_fs(const char* fileImage){
    
    if(!fileImage){                                                                             // Checking whether the file image is found                                                 
        handle_error_ret("\n#### ERROR! File image not found! ####\n", -1);
    }

    int fs_fd = open(fileImage, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);                                           // Opening the file image in read-write mode                           
    if(fs_fd == -1){                                                                                           
        handle_error_ret("\n#### ERROR! Couldn't open the fileImage! ####\n", -1);
    }

    if(ftruncate(fs_fd, SIZE) == -1){                                                              // Truncating the file image to the specified size
        free_fs();                                                                              // Freeing the file system
        handle_error_ret("\n#### ERROR! Couldn't truncate the fileImage! ####\n", -1);
    }         

    void* fs_start = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , fs_fd, 0);                     // Mapping the file image to the memory and setting the starting address of the file system
    if(fs_start == MAP_FAILED){          
        free_fs();                                                                              // Freeing the file system                                                                  
        handle_error_ret("\n#### ERROR! Couldn't map the fileImage! ####\n", -1);
    }

    file_fs = fdopen(fs_fd, "wb+");
    if (!file_fs) {
        printf("Error creating file system file\n");
        munmap(fs_start, SIZE);
        close(fs_fd);
        return -1;
    }

    fs = (FileSystem*)fs_start;                                                                 // Setting the file system pointer to the starting address of the file system
    fs->bytes_per_block = SECTOR_SIZE;
    fs->total_blocks = TOTAL_SECTORS;
    fs->fat_entries = TOTAL_SECTORS;
    fs->fat_size = fs->fat_entries * sizeof(int);
    fs->data_size = SIZE - fs->fat_size - sizeof(FileSystem);
    strcpy(fs->current_directory, "ROOT");

    fat = (int*)((char*)fs_start + sizeof(FileSystem));
    for (int i = 0; i < fs->fat_entries; i++) {
        fat[i] = 0;
    }

    data_blocks = (char*)fs_start + sizeof(FileSystem) + fs->fat_size;
    memset(data_blocks, 0, fs->data_size);                                     // we set the start of the root directory at the start of the File System + the size of the FAT

    // 0 means that the block is free
    // -1 means that the block is reserved
    // -2 means that the block is the last block in the file (EOF)                                                               // we set the FAT size to the number of elements in the FAT

    current_directory = (DirectoryElement*) data_blocks;                                                                   // we set the current directory to the root directory
    current_directory->is_directory = 1;                                                        // we set the current directory to a directory
    current_directory->start_block = 0;
    fat[current_directory->start_block] = -1;                                                   // we set the start block of the current directory in the FAT to -1
    strncpy(current_directory->name, "ROOT", MAX_FILE_NAME);                                    // we set the name of the current directory to ROOT
    current_directory->parent = NULL;                                              // we set the parent directory to the root directory

    fat[0] = -1;                                                                                // we set the start block of the root directory in the FAT to -1

    return 0;                                                                                                                                   
}


int load_fs(const char *filename) {
    
    int fs_fd = open(filename, O_RDWR);
    if (fs_fd == -1) {
        printf("Error opening file system file\n");
        return -1;
    }

    void* fs_start = mmap(NULL, TOTAL_SECTORS*CLUSTER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fs_fd, 0);
    if (fs_start == MAP_FAILED) {
        printf("Error mapping file\n");
        close(fs_fd);
        return -1;
    }

    file_fs = fdopen(fs_fd, "wb+");
    if (!file_fs) {
        printf("Error creating file system file\n");
        munmap(fs_start, SIZE);
        close(fs_fd);
        return -1;
    }

    fs = (FileSystem*)fs_start;
    fat = (int*)((char*)fs_start + sizeof(FileSystem));
    data_blocks = (char*)fs_start + sizeof(FileSystem) + fs->fat_size;
    current_directory = (DirectoryElement*)data_blocks;

    printf("fs_load: PASSED\n");
    printf("fs_load: Loaded file system from data file.\n");

    return 1;
}

int save_fs() {

    if (msync(fs, TOTAL_SECTORS*CLUSTER_SIZE, MS_SYNC) == -1) {
        printf("fs_save: Failed to sync memory to file\n");
        return -1;
    }

    printf("fs_save: PASSED\n");
    printf("fs_save: Successfully saved file system to DATATICUS file.\n");

    return 0;
}

void print_fat(int items){
    if(items > fs->fat_size){
        items = fs->fat_size;
    }
    for(int i = 0; i < items; i++){
        printf("FAT[%d]: %d\n", i, fat[i]);
    }
}

int free_fat_block(){                                                                           // Function to get a free block in the FAT
    for(int i = 2; i < fs->fat_size; i++){                                                          // we start from 2 because the first two blocks are reserved to the current directory
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

        FileHandler *fh = (FileHandler*)malloc(sizeof(FileHandler));                                                                     // Allocating memory for the file handler
        if(!fh){
            handle_error_ret("\n#### ERROR! Couldn't allocate memory for the file handler! ####\n", NULL);
        }

        fh->size = 0;                                                                                                                   // Setting the size of the file to 0
        fh->pos = 0;                                                                                                                    // Setting the current position in the file to 0
        fh->directory = current_directory;                                                                                              // Setting the directory of the file handler to the current directory
        strncpy(fh->file_name, name, MAX_FILE_NAME-1);                                                                                  // Copying the name to the file handler
        fh->file_name[MAX_FILE_NAME-1] = '\0';

    
        for(int i = 0; i < current_directory->size; i++){                                                                                // Searching for a free block in the current directory
            if(current_directory[i].size == 0 && current_directory[i].name[0] == '\0' && current_directory[i].is_directory == 0){
                strncpy(current_directory[i].name, name, MAX_FILE_NAME-2);                                                               // Copying the name we passed to the name section of the file in the directory
                current_directory[i].name[MAX_FILE_NAME-1] = '\0';
                current_directory[i].start_block = free_block;                                                                           // Setting the start block of the file in the FAT to the free block we got
                fat[free_block] = -2;                                                                                                    // Setting the block in the FAT to -2 (end of file)
                current_directory->size = sizeof(DirectoryElement)*i;                                                                    // Updating the size of the current directory
                current_directory[i].is_directory = 0;                                                                                   // Setting the file in the directory to a file
                current_directory[i].fh = fh;                                                                                             // Setting the file handler of the file in the directory to the file handler we created
                current_directory[i].pos = 0;                                                                                             // Setting the current position in the file to 0
                break;
            }
        }

        return fh;
}

int erase_file(const char *name){
    
    if(!name){
        handle_error_ret("\n#### ERROR! File name not found! ####\n", -1);
    }

    for(int i = 1; i < current_directory->size; i++){                                       // Searching for the file in the current directory
        if(strcmp(current_directory[i].name, name) == 0){                                   // If the file is found
            int block = current_directory[i].start_block;                                   // Getting the start block of the file in the FAT
            printf("Erasing file: %s\n", name);

            while(fat[block] != 0){                                                         // while the block is not empty
                if(fat[block] == -2){                                                       // we reached the end of the file
                    fat[block] = 0;                                                         // we set the block to 0
                    printf("Freed block: %u\n", block);
                    break;
                }
                int next_block = fat[block];                                                // we traverse the FAT to the next block
                fat[block] = 0;
                printf("Freed block: %u\n", block);
                block = next_block;
            }

            memset(&current_directory[i], 0, sizeof(DirectoryElement));                     // we empty the file in the directory
            printf("File %s erased successfully!\n", name);
            return 0;
        }
    }

    return -1;
}


int write_file(FileHandler *fh, const char *data){

    int newblock = 0;

    if(!fh){
        handle_error_ret("\n#### ERROR! File handler not found! ####\n", -1);
    }

    if(!data){
        handle_error_ret("\n#### ERROR! Data not found! ####\n", -1);
    }

    long int data_size = strlen(data);                                                            // Getting the size of the data
    int data_pos = fh->pos % CLUSTER_SIZE;                                                                  // Getting the current position in the file
    int rem_size = data_size;                                                                // Getting the remaining size of the data
    int bytes_written = 0;                                                                   // Number of bytes written to the file
    DirectoryElement *file = NULL;

    int is_in_current_directory = 0;

    if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){          // Checking whether the file is in the current directory
        is_in_current_directory = 1;
    }

    if(is_in_current_directory){
        for(int i = 0; i < current_directory->size; i++){                                                                           // Searching for the file in the current directory
            if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){
                file = &current_directory[i];                                                                                       // If the file is found, we set the file pointer to the file in the directory                   
                break;
            }
        }
    }else{
        handle_error_ret("\n#### ERROR! File not found in the current directory! ####\n", -1);
    }

    int block = file->start_block;                                                         // Getting the start block of the file in the FAT 

    while(rem_size > 0){

        if(rem_size > CLUSTER_SIZE){                      
            newblock = 1;
        }

        if(fat[block] == -2 && fh->size == 0){                                                              // if the file is empty
            int new_block = free_fat_block();
            if(new_block == -1){
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            fat[block] = new_block;
            file->current_block = block;
            fat[new_block] = -2;
        }


        if(newblock){                                       // If the data position is a multiple of the cluster size and not 0, then we set the block to the next block in the FAT
            char* block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block));                     // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
            memcpy(block_data, data, CLUSTER_SIZE);
            int new_block = free_fat_block();
            if(new_block == -1){
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            int next_block = fat[file->current_block];
            fat[next_block] = new_block;
            fat[new_block] = -2;
            file->current_block = next_block;
            newblock = 0;

            bytes_written += CLUSTER_SIZE;
            rem_size -= bytes_written;
            fh->pos += bytes_written;   
            file->size += bytes_written;                                                                          // Updating the size of the file                                                                        // Updating the current position in the file

        }else{
            char* block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block));                     // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
            memcpy(block_data + data_pos, data, rem_size);
            printf("BLOCK DATA WRITE: %s\n", block_data);
            bytes_written += rem_size;
            rem_size -= bytes_written;
            fh->pos += bytes_written;   
            file->size += bytes_written;                                                                          // Updating the size of the file
        }  

    }

    fh->size = file->size;
    printf("FH SIZE: %d\n", fh->size);
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
    int rem_size = buff_size-data_pos;                                                             // Getting the remaining size of the buffer
    int bytes_read = 0;                                                                   // Number of bytes read from the file
    int newblock = 0;                                                                    // Flag to check whether we need to move to the next block in the FAT
    DirectoryElement *file = NULL;

    int is_in_current_directory = 0;
    
    if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){        // we perform the same check as in the write_file function
        is_in_current_directory = 1;
    }

    if(is_in_current_directory){
        for(int i = 0; i < current_directory->size; i++){                                                   
            if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){
                file = &current_directory[i];
                file->current_block = file->start_block;                                                                            // Setting the current block of the file to the start block of the file in the FAT
                break;
            }
        }
    }else{
        handle_error_ret("\n#### ERROR! File not found in the current directory! ####\n", -1);
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int block = file->start_block;                                                          // Getting the start block of the file in the FAT

    while(rem_size > 0 && fat[block] != -2){    
        printf("rem_size: %d\n", rem_size);                                                 // While the remaining size of the buffer is greater than 0 and the block is not the last block in the file

        if(rem_size > CLUSTER_SIZE){                      
            newblock = 1;
        }

        if(newblock){                                                        // If the remaining size of the buffer is less than the cluster size
            char *block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block));
            memcpy(buff+bytes_read, block_data, CLUSTER_SIZE); 
            bytes_read += CLUSTER_SIZE;         
            rem_size -= bytes_read;                                                        
            file->current_block = fat[file->current_block];
            newblock = 0;
        }else{
            char *block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block));
            memcpy(buff+bytes_read, block_data + data_pos, rem_size);
            if(data_pos % CLUSTER_SIZE == 0){
                bytes_read += rem_size;
            }else{
                bytes_read += data_pos;         
            }
            rem_size -= bytes_read;                                                        
        }
    
    }

    printf("Bytes read: %d\n", bytes_read);

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

    int is_in_current_directory = 0;

    if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){          // Checking whether the file is in the current directory
        is_in_current_directory = 1;
    }

    if(is_in_current_directory){
        for(int i = 0; i < current_directory->size; i++){                                                                           // Searching for the file in the current directory
            if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){
                file = &current_directory[i];                                                                                       // If the file is found, we set the file pointer to the file in the directory                   
                break;
            }
        }
    }else{
        handle_error_ret("\n#### ERROR! File not found in the current directory! ####\n", -1);
    }


    printf("file->size: %d\n", file->size);
    printf("pos: %d\n", pos);

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    if(pos > file->size){
        handle_error_ret("\n#### ERROR! Position is greater than the file size! ####\n", -1);
    }
    file->pos = pos;                                                                       // Updating the current position in the file       
    fh->pos = pos;                                                                         // Updating the current position in the file handler            
    return 0;
}


int list_directory(){
    
    printf("\n#### LISTING DIRECTORY %s ####\n", current_directory->name);
    for(int i = 1; i < current_directory->size; i++){                                   // Listing the files in the current directory
        if(current_directory[i].name[0] != '\0'){                                       // Checking whether the name is not empty
            if(current_directory[i].is_directory){                                      // If the file is a directory                 
                printf("Directory: %s\n", current_directory[i].name);
            }else{                                                                      // If the file is not a directory
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

    for (int i = 0; i < current_directory->size; i++){                                                  // Checking whether the name is already in use in the current directory
        if (strcmp(current_directory[i].name, name) == 0){
            handle_error_ret("\n#### ERROR! Directory with this name already exists! ####\n", -1);
        }
    }

    int free_block = free_fat_block();                                                                  // Getting a free block in the FAT
    if (free_block == -1){
        handle_error("\n#### ERROR! No free blocks in the FAT! ####\n");
        return -1;
    }

    int found_empty_slot = 0;
    for (int i = 0; i < current_directory->size; i++){
        if (current_directory[i].size == 0 && current_directory[i].name[0] == '\0'){                    // Searching for a free slot in the current directory
            strncpy(current_directory[i].name, name, MAX_FILE_NAME - 1);                                // Copying the name we passed to the name section of the directory
            current_directory[i].name[MAX_FILE_NAME - 1] = '\0';
            current_directory[i].start_block = free_block;                                              // Setting the start block of this new directory in the FAT to the free block we got
            current_directory[i].is_directory = 1;
            fat[free_block] = -1;                                                                       // Setting the block in the FAT to -1 (reserved)
            current_directory[i].size = sizeof(DirectoryElement);
            current_directory[i].current_block = free_block;
            current_directory[i].parent = current_directory;
            found_empty_slot = 1;
            break;
        }
    }

    if (!found_empty_slot){
        handle_error_ret("\n#### ERROR! No empty slot for new directory! ####\n", -1);
    }

    current_directory->size += sizeof(DirectoryElement);

    return 0;
}



int change_directory(const char *name) {

    if(!name){
        handle_error_ret("\n#### ERROR! Directory name not found! ####\n", -1);
    }

    if(strcmp(name, current_directory->name) == 0){
        handle_error_ret("\n#### ERROR! Already in the directory! ####\n", -1);
    }

    if(strcmp(name, "...") == 0){
        current_directory = parent_directory;
        return 0;
    }
    

    if(strcmp(name, "..") == 0){                                                                      // Checking whether the name is "..", if so we change the directory to the parent directory (root)
        if (current_directory == parent_directory) {
            handle_error_ret("\n#### ERROR! Already in the root directory! ####\n", -1);
        }


        current_directory = current_directory->parent;                                                  // Setting the current directory to the parent directory

        return 0;
    }



    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory) {        // Searching for the directory in the current directory
            int block = current_directory[i].start_block;                                               // Getting the start block of the directory in the FAT
            DirectoryElement *par = current_directory;
            current_directory = (DirectoryElement *)(fs_start + CLUSTER_SIZE * block);                  // Setting the current directory to the new directory
            strncpy(current_directory->name, name, MAX_FILE_NAME - 1);                                  // Copying the name of the new directory to the current directory
            current_directory->size = sizeof(DirectoryElement);
            current_directory->is_directory = 1;
            current_directory->parent = par;
            current_directory->start_block = block;
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

    if(change_directory(name) == -1){
        handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
    }

    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory) {
            for (int j = current_directory->size; j > 0; j--) {                                                        // Erasing the files in the directory
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

    current_directory = current_directory->parent;

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

FileHandler *get_file_handler(const char *name) {
    if (!name) {
        handle_error_ret("\n#### ERROR! File name not found! ####\n", NULL);
    }

    for (int i = 0; i < current_directory->size; i++) {
        if (strcmp(current_directory[i].name, name) == 0 && current_directory[i].is_directory == 0) {
            return current_directory[i].fh;
        }
    }

    handle_error_ret("\n#### ERROR! File not found! ####\n", NULL);
}

void free_fs(){
    if(fat){
        munmap(fat, fat_size * 4);
        printf("FAT unmapped!\n");
    }
    if(fs_start){
        munmap(fs_start, SIZE);
        printf("File system unmapped!\n");
    }
}

