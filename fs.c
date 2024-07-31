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

    //  0 means that the block is free
    // -1 means that the block is reserved
    // -2 means that the block is the last block in the file (EOF)   
    // -3 FAT FULL

    current_directory = (DirectoryElement*) data_blocks;                                                                   // we set the current directory to the root directory
    current_directory->is_directory = 1;                                                        // we set the current directory to a directory
    current_directory->start_block = 0;
    fat[current_directory->start_block] = -1;                                                   // we set the start block of the current directory in the FAT to -1
    strncpy(current_directory->name, "ROOT", MAX_FILE_NAME);                                    // we set the name of the current directory to ROOT
    current_directory->parent = NULL;                                              // we set the parent directory to the root directory

    fat[0] = -2;                                                                                // we set the start block of the root directory in the FAT to -1

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

DirectoryElement* empty_dir_element() {
    int block = current_directory->start_block;
    while (block != -3) {
        DirectoryElement* dir = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
        for (int i = 0; i < fs->bytes_per_block / sizeof(DirectoryElement); i++) {
            DirectoryElement* entry = &dir[i];
            if (entry->name[0] == 0x00 || (unsigned char)entry->name[0] == 0xFF) {
                return entry;
            }
        }
        block = fat[block];
    }
    return NULL;
}

DirectoryElement* locate_file(const char* name, char is_dir) {
    int block = current_directory->start_block;
    while (block != -3) {
        DirectoryElement* dir = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
        for (int i = 0; i < fs->bytes_per_block / sizeof(DirectoryElement); i++) {
            DirectoryElement* entry = &dir[i];
            if (strcmp(entry->name, name) == 0 && entry->is_directory == is_dir) {
                return entry;
            }
        }
        block = fat[block];
    }
    return NULL;
}

int create_file(const char *name, int size, const char* data){
    
        if(!name){
            handle_error_ret("\n#### ERROR! File name not found! ####\n", -1);
        }

        if(strlen(name) > MAX_FILE_NAME){
            handle_error_ret("\n#### ERROR! File name is too long! ####\n", -1);
        }

        DirectoryElement* entry = empty_dir_element();
        if (entry == NULL) {
            handle_error_ret("\n#### ERROR! No free directory entries! ####\n", -1);
        }

        memset(entry->name, ' ', sizeof(entry->name));
    
        strcpy(entry->name, name);
        entry->name[MAX_FILE_NAME-1] = '\0';
        entry->size = size;
        entry->is_directory = 0;

        int block = free_fat_block();
        if (block == -3) {
            handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
        }

        entry->start_block = block;
        entry->parent = current_directory;
        
        int blk_size = fs->bytes_per_block;
        int blk_needed = (size + blk_size - 1) / blk_size;
        int curr_blk = block;

        for (int i = 0; i < blk_needed; i++) {
            if (curr_blk >= fs->fat_entries) {
                handle_error_ret("\n#### ERROR! File creation error! ####\n", -1);
            }

            memcpy(&data_blocks[curr_blk * blk_size], &data[i * blk_size], blk_size);
            int next_blk = free_fat_block();
            if (i == blk_needed - 1) {
                fat[curr_blk] = -2;
            } else {
                fat[curr_blk] = next_blk;
            }
            curr_blk = next_blk;
        }

        save_fs();

        return 0;
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


int write_file(const char* name, const char* data) {
    
    int size = strlen(data);
    int data_pos = 0;
    int rem_size = size;
    int bytes_written = 0;
    printf("Received %d bytes to write to file '%s'\n", size, name); 

    DirectoryElement* entry = locate_file(name, 0);
    if(entry == NULL) {
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int blk_size = fs->bytes_per_block;
    int curr_blk = entry->start_block;
    int tot_blk = fs->fat_entries;
    int offset = data_pos % blk_size;

    int* written_blocks = (int*)calloc(tot_blk, sizeof(int));
    if (written_blocks == NULL) {
        handle_error_ret("\n#### ERROR! Memory allocation error! ####\n", -1);
    }

    while (bytes_written < rem_size) {
        if (written_blocks[curr_blk] == 1) {
            int next_block = fat[curr_blk];
            if (next_block == -2 || next_block == 0) {
                int new_block = free_fat_block();
                if (new_block == -3) {
                    free(written_blocks);
                    handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
                }
                fat[curr_blk] = new_block;
                fat[new_block] = -2;
                curr_blk = new_block;
            } else {
                curr_blk = next_block;
            }
            continue;
        }

        written_blocks[curr_blk] = 1;

        if (curr_blk == -2 || curr_blk == 0) {
            int new_block = free_fat_block();
            if (new_block == -3) {
                free(written_blocks);
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            fat[curr_blk] = new_block;
            fat[new_block] = -2;
            curr_blk = new_block;
        }

        if (curr_blk == -4 || curr_blk >= fs->fat_entries || curr_blk == 0) {
            printf("write_file_content: Error - current_block %d is out of bounds, unused, or invalid\n", curr_blk);
            free(written_blocks);
            handle_error_ret("\n#### ERROR! File write error! ####\n", -1);
        }

        int bytes_to_write = (rem_size - bytes_written > blk_size - data_pos) ? blk_size - data_pos : rem_size - bytes_written; 
        memcpy(&data_blocks[curr_blk * blk_size + data_pos], data + bytes_written, bytes_to_write);

        bytes_written += bytes_to_write;
        data_pos += bytes_written;

        if (bytes_written < rem_size) {
            int next_block = fat[curr_blk];
            if (next_block == -2 || next_block == 0) {
                int new_block = free_fat_block();
                if (new_block == -3) {
                    free(written_blocks);
                    handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
                }
                fat[curr_blk] = new_block;
                fat[new_block] = -2;
                curr_blk = new_block;
            } else {
                curr_blk = next_block;
            }
        }

    }

    entry->size = data_pos + bytes_written > entry->size ? offset + bytes_written : entry->size;

    save_fs();

    printf("write_file_content: Written %d bytes to file '%s'\n", bytes_written, name);

    free(written_blocks);
    return bytes_written;

}

int read_file(FileHandler *fh, char*buff, int buff_size){

    if(!fh || !buff || buff_size <= 0){
        handle_error_ret("\n#### ERROR! Invalid read parameters! ####\n", -1);
    }

    DirectoryElement* entry = fh->file_entry;
    int bytes_read = 0;
    int total_size = entry->size;
    int current_block = entry->start_block;
    int byte_offset = fh->pos % SECTOR_SIZE;

    if (fh->pos >= total_size) {
        buff[0] = '\0';
        return 0;
    }

    int blocks_to_skip = fh->pos / SECTOR_SIZE;
    for (int i = 0; i < blocks_to_skip; i++) {
        current_block = fat[current_block];
        if (current_block == -2) {
            return bytes_read;
        }
    }

    while (bytes_read < buff_size && fh->pos < total_size) {
        int bytes_to_copy = SECTOR_SIZE - byte_offset;
        if (bytes_read + bytes_to_copy > buff_size) {
            bytes_to_copy = buff_size - bytes_read;
        }
        if (fh->pos + bytes_to_copy > total_size) {
            bytes_to_copy = total_size - fh->pos;
        }

        if (current_block >= fs->fat_entries || current_block == -4 || current_block == 0) {
            handle_error_ret("\n#### ERROR! File read error! ####\n", -1);
        }

        printf("read_file_content: Reading %d bytes from block %d\n", bytes_to_copy, current_block); 
        memcpy(buff + bytes_read, data_blocks + current_block * SECTOR_SIZE + byte_offset, bytes_to_copy);
        bytes_read += bytes_to_copy;
        fh->pos += bytes_to_copy;
        byte_offset = 0;

        if (fh->pos < total_size) {
            int next_block = fat[current_block];
            if (next_block == -2) {
                break;
            }
            if (next_block >= fs->fat_entries || next_block == -4 || next_block == 0) {
                handle_error_ret("\n#### ERROR! File read error! ####\n", -1);
            }
            current_block = next_block;
        }
    }

    if (bytes_read < buff_size) {
        buff[bytes_read] = '\0';
    } else {
        buff[buff_size - 1] = '\0';
    }

    printf("File content:\n%s\n", buff); 

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

