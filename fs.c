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
        handle_error_ret("\n#### ERROR! Couldn't truncate the fileImage! ####\n", -1);
    }         

    void* fs_start = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED , fs_fd, 0);                     // Mapping the file image to the memory and setting the starting address of the file system
    if(fs_start == MAP_FAILED){          
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
        fat[i] = FAT_UNUSED;
    }

    data_blocks = (char*)fs_start + sizeof(FileSystem) + fs->fat_size;
    memset(data_blocks, 0x00, fs->data_size);                                     // we set the start of the root directory at the start of the File System + the size of the FAT

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

    fat[0] = FAT_END;                                                                                // we set the start block of the root directory in the FAT to -1

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

    printf("- Loaded file system from data file.\n");

    return 0;
}

int save_fs() {

    if (!file_fs) {
        handle_error_ret("\n#### ERROR! File system file not found! ####\n", -1);
    }

    if (msync(fs, TOTAL_SECTORS*SECTOR_SIZE, MS_SYNC) == -1) {
        printf("- Failed to sync memory to file\n");
        return -1;
    }

    printf("- Successfully saved file system to data file.\n");

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
    for(int i = 1; i < fs->fat_size; i++){                                                          // we start from 2 because the first two blocks are reserved to the current directory
        if(fat[i] == FAT_UNUSED){
            return i;
        }
    }
    return FAT_FULL;
}

DirectoryElement* empty_dir_element() {
    int block = current_directory->start_block;
    while (block != FAT_END) {
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
    while (block != FAT_END) {
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
        if (block == FAT_FULL) {
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
                fat[curr_blk] = FAT_END;
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
    
    printf("- Attempting to remove file: %s\n", name);
    DirectoryElement* file = locate_file(name, 0);
    if (file == NULL) {
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int current_block = file->start_block;
    int next_block;

    while (current_block != FAT_END && current_block != 0) {

        next_block = fat[current_block];
        if (next_block < 0 || next_block >= fs->fat_entries) {
            fat[current_block] = FAT_UNUSED;
            memset(&data_blocks[current_block * fs->bytes_per_block], 0x00, fs->bytes_per_block);
            break;
        }

        fat[current_block] = FAT_UNUSED;
        memset(&data_blocks[current_block * fs->bytes_per_block], 0x00, fs->bytes_per_block);

        if (next_block == FAT_END || next_block == 0) {
            break;
        }
        current_block = next_block;
    }

    file->name[0] = 0xFF;

    save_fs();

    return 0;
}


int write_file(FileHandler* fh, const char* data) {    

    if(fh->file_entry->size > 0 && fh->pos == 0){
        seek_file(fh, fh->file_entry->size);
    }

    int size = strlen(data);
    int data_pos = fh->pos;
    int rem_size = size;
    int bytes_written = 0;
    printf("- Received %d bytes to write to file '%s'\n", size, fh->file_entry->name); 

    int blk_size = fs->bytes_per_block;
    int curr_blk = fh->file_entry->start_block;
    int tot_blk = fs->fat_entries;
    int offset = data_pos % blk_size;

    int* written_blocks = (int*)calloc(tot_blk, sizeof(int));
    if (written_blocks == NULL) {
        handle_error_ret("\n#### ERROR! Memory allocation error! ####\n", -1);
    }

    while (bytes_written < rem_size) {
        if (written_blocks[curr_blk] == 1) {
            int next_block = fat[curr_blk];
            if (next_block == FAT_END || next_block == 0) {
                int new_block = free_fat_block();
                if (new_block == FAT_FULL) {
                    free(written_blocks);
                    handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
                }
                fat[curr_blk] = new_block;
                fat[new_block] = FAT_END;
                curr_blk = new_block;
            } else {
                curr_blk = next_block;
            }
            continue;
        }

        written_blocks[curr_blk] = 1;

        if (curr_blk == FAT_END || curr_blk == 0) {
            int new_block = free_fat_block();
            if (new_block == FAT_FULL) {
                free(written_blocks);
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            fat[curr_blk] = new_block;
            fat[new_block] = FAT_END;
            curr_blk = new_block;
        }

        if (curr_blk == FAT_UNUSED || curr_blk >= fs->fat_entries || curr_blk == 0) {
            free(written_blocks);
            handle_error_ret("\n#### ERROR! File write error! ####\n", -1);
        }

        int bytes_left = (rem_size - bytes_written > blk_size - data_pos) ? blk_size - data_pos : rem_size - bytes_written; 
        memcpy(&data_blocks[curr_blk * blk_size + data_pos], data + bytes_written, bytes_left);

        bytes_written += bytes_left;
        data_pos = 0;

        if (bytes_written < rem_size) {
            int next_block = fat[curr_blk];
            if (next_block == FAT_END || next_block == 0) {
                int new_block = free_fat_block();
                if (new_block == FAT_FULL) {
                    free(written_blocks);
                    handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
                }
                fat[curr_blk] = new_block;
                fat[new_block] = FAT_END;
                curr_blk = new_block;
            } else {
                curr_blk = next_block;
            }
        }

    }

    fh->file_entry->size = data_pos + bytes_written > fh->file_entry->size ? offset + bytes_written : fh->file_entry->size;

    save_fs();

    printf("- Written %d bytes to file '%s'\n", bytes_written, fh->file_entry->name);

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
        if (current_block == FAT_END) {
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

        if (current_block >= fs->fat_entries || current_block == FAT_UNUSED || current_block == 0) {
            handle_error_ret("\n#### ERROR! File read error! ####\n", -1);
        }

        memcpy(buff + bytes_read, data_blocks + current_block * SECTOR_SIZE + byte_offset, bytes_to_copy);
        bytes_read += bytes_to_copy;
        fh->pos += bytes_to_copy;
        byte_offset = 0;

        if (fh->pos < total_size) {
            int next_block = fat[current_block];
            if (next_block == FAT_END) {
                break;
            }
            if (next_block >= fs->fat_entries || next_block == FAT_UNUSED || next_block == 0) {
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

    printf("- File content:\n  %s\n", buff); 

    return bytes_read;

}

int seek_file(FileHandler *fh, int pos){

    if (pos < 0 || pos > fh->file_entry->size) {
        return -1; 
    }

    fh->pos = pos;
    return 0;
}

int list_directory(){

    if (current_directory == NULL) {
        handle_error_ret("\n#### ERROR! Current directory not found! ####\n", -1);
    }

    printf("\n#### LISTING DIRECTORY %s ####\n", current_directory->name);

    int block = current_directory->start_block;
    while (block != FAT_END) {
        DirectoryElement* dir = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
        for (int i = 0; i < fs->bytes_per_block / sizeof(DirectoryElement); i++) {
            DirectoryElement* entry = &dir[i];
            if (entry->name[0] == 0x00) {
                continue;
            }
            if ((unsigned char)entry->name[0] == 0xFF) {
                continue;
            }
            if (entry->is_directory) {
                printf("Directory: %s\n", entry->name);
            } else {
                printf("File: %s\n", entry->name);
            }
        }
        int next_block = fat[block];
        if (next_block == FAT_UNUSED || next_block == FAT_END || next_block < 0 || next_block >= fs->fat_entries) {
            break;
        }
        block = next_block;
    }
    printf("\n");

    return 0;
}

int create_directory(const char *name){

    printf("- Creating directory: %s\n", name);

    DirectoryElement* entry = empty_dir_element();
    if (entry == NULL) {
        handle_error_ret("\n#### ERROR! No free directory entries! ####\n", -1);
    }

    if(strlen(name) > MAX_FILE_NAME){
        handle_error_ret("\n#### ERROR! Directory name is too long! ####\n", -1);
    }

    memset(entry->name, ' ', sizeof(entry->name));
    strncpy(entry->name, name, sizeof(entry->name) - 1); 
    entry->name[sizeof(entry->name) - 1] = '\0';
    entry->size = 0;
    entry->is_directory = 1;

    int block = free_fat_block();
    if (block == FAT_FULL) {
        handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
    }

    fat[block] = FAT_END;
    DirectoryElement* new_dir = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
    memset(new_dir, 0, fs->bytes_per_block);

    entry->start_block = block;
    strcpy(new_dir[0].name, name);
    new_dir[0].name[sizeof(new_dir[0].name) - 1] = '\0';
    new_dir[0].is_directory = 1;
    new_dir[0].size = 0;
    new_dir[0].start_block = block;
    new_dir[0].parent = current_directory;

    strcpy(new_dir[1].name, "..");
    new_dir[1].name[sizeof(new_dir[1].name) - 1] = '\0';
    new_dir[1].is_directory = 1;
    new_dir[1].size = 0;
    new_dir[1].start_block = current_directory->start_block;
    new_dir[1].parent = current_directory;

    save_fs();

    return 0;
}

int erase_directory(const char *name){

    printf("- Attempting to remove directory: %s\n", name);
    DirectoryElement* dir = locate_file(name, 1);
    if (dir == NULL) {
        handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
    }

    change_directory(name);

    int block = dir->start_block;
    while (block != FAT_END){
        DirectoryElement* dirCheck = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
        for(int i = 0; i < fs->bytes_per_block / sizeof(DirectoryElement); i++){
            DirectoryElement* entry = &dirCheck[i];
            if(entry->name[0] == 0x00){
                continue;
            }
            if((unsigned char)entry->name[0] == 0xFF){
                continue;
            }
            if(entry->is_directory != 1){
                erase_file(entry->name);
            }
        }
        block = fat[block];
    }

    int curr_blk = dir->start_block;

    while(curr_blk != FAT_END){
        memset(&data_blocks[curr_blk * fs->bytes_per_block], 0x00, fs->bytes_per_block);
        int next_block = fat[curr_blk];
        fat[curr_blk] = FAT_UNUSED;
        curr_blk = next_block;
    }

    memset(dir, 0x00, sizeof(DirectoryElement));

    save_fs();

    return 0;
}

int change_directory(const char *name){

    if (strcmp(name, current_directory->name) == 0) {
        handle_error_ret("\n#### ERROR! Cannot change to current directory! ####\n\n", -1);
    }

    if (strcmp(name, "..") == 0) {
        if (current_directory->parent != NULL) {
            current_directory = current_directory->parent;
            strcpy(fs->current_directory, current_directory->name);
        }
        return 0;
    }

    int block = current_directory->start_block;
    while (block != FAT_END) {
        DirectoryElement* dir = (DirectoryElement*)&data_blocks[block * fs->bytes_per_block];
        for (int i = 0; i < fs->bytes_per_block / sizeof(DirectoryElement); i++) {
            DirectoryElement* entry = &dir[i];
            if (strcmp(entry->name, name) == 0 && entry->is_directory) {
                current_directory = (DirectoryElement*)&data_blocks[entry->start_block * fs->bytes_per_block];
                current_directory->parent = dir;
                strcpy(fs->current_directory, entry->name);
                return 0;
            }
        }
        block = fat[block];
        if (block < 0 || block >= fs->fat_entries) {
            handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
        }
    }

    handle_error_ret("\n#### ERROR! Directory not found! ####\n", -1);
}
