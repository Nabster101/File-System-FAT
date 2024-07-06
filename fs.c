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

    // 0 means that the block is free
    // -1 means that the block is reserved
    // -2 means that the block is the last block in the file (EOF)

    memset(fat, 0, FAT_ELEMENTS * 4);   
    printf("FAT initialized!\n");
    
    memset(root, 0, ((size - CLUSTER_SIZE) / sizeof(DirectoryElement) * sizeof(DirectoryElement)));
    printf("Root initialized!\n");

    root_size = (size - CLUSTER_SIZE) / sizeof(DirectoryElement);
    fat_size = FAT_ELEMENTS;
    
    return 0;                                                                                                                                   
}

int free_fat_block(){
    
    for(int i = 0; i < fat_size; i++){
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
    
        for(int i = 0; i < root_size; i++){                                                                                     // Checking whether the name is already in use
            if(strcmp(root[i].name, name) == 0){
                handle_error_ret("\n#### ERROR! File with this name already exists! ####\n", NULL);
            }
        }
    
        int free_block = free_fat_block();    
        if(free_block == -1){
            handle_error("\n#### ERROR! No free blocks in the FAT! ####\n");
            return NULL;
        }
    
        for(int i = 0; i < root_size; i++){                                                                                      // Searching for a free block in the root
            if(root[i].size == 0 && root[i].name[0] == '\0'){
                strncpy(root[i].name, name, MAX_FILE_NAME-2);
                root[i].name[MAX_FILE_NAME-1] = '\0';
                root[i].start_block = free_block;
                fat[free_block] = -1;
                root[i].size = 0;
                break;
            }
        }
    
        FileHandler *fh = (FileHandler*)malloc(sizeof(FileHandler));                                                             // Allocating memory for the file handler
        if(!fh){
            handle_error("\n#### ERROR! Couldn't allocate memory for the file handler! ####\n");
            return NULL;
        }
    
        fh->pos = 0;
        fh->directory = root;
    
        printf("\n#### HOLD UP! File %s created successfully! ####\n", name);
    
        return fh;
}

int erase_file(const char *name){
    
    if(!name){
        handle_error_ret("\n#### ERROR! File name not found! ####\n", -1);
    }

    for(int i = 0; i < root_size; i++){
        if(strcmp(root[i].name, name) == 0){

            int block = root[i].start_block;
            printf("Erasing file: %s\n", name);
            while(block != -1){
                int next_block = fat[block];
                fat[block] = -2;
                printf("Freed block: %u\n", block);
                block = next_block;
            }

            memset(&root[i], 0, sizeof(DirectoryElement));
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
    int block_offset = data_pos / CLUSTER_SIZE;                                              // Getting the block offset (block number where the data is stored relative to the starting block of the file)
    int byte_offset = data_pos % CLUSTER_SIZE;                                               // Getting the byte offset (byte number in the block where the data is stored)
    int rem_size = data_size;                                                                // Getting the remaining size of the data
    int bytes_written = 0;                                                                   // Number of bytes written to the file

    DirectoryElement *file = NULL;

    for(int i = 0; i < root_size; i++){                                                      // Searching for the file in the root
        if(strncmp(root[i].name, fh->directory->name, MAX_FILE_NAME) == 0){
            file = &root[i];
            break;
        }
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int block = file->start_block;                                                         // Getting the start block of the file in the FAT                                           
    int prev_block = -1;                                                                   // Previous block in the FAT (used for linking the blocks)                  

    for(int i = 0; i < block_offset; i++){                                                 // Traversing the FAT to the block offset (block number where the data is stored relative to the starting block of the file)
        if(fat[block] == -2){
            handle_error_ret("\n#### ERROR! Block offset is out of bounds! ####\n", -1);   // If the block is -2, then the block offset is out of bounds
        }
        prev_block = block;                                                                // Updating the previous block to the current block (used for linking the blocks)
        block = fat[block];
    }                                                                                      // with this loop, we are traversing the FAT to the block where the fh->pos is pointing to

    while(rem_size > 0){

        if(fat[block] == -2 || fat[block] == 0){                                           // If the block is -2 or 0 in the FAT, then we need to allocate a new block in the FAT
            int new_block = free_fat_block();                                              // Getting a free block in the FAT
            if(new_block == -1){
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }

            if(prev_block == -1){                                                          // If the previous block is -1, then the new block is the start block of the file
                file->start_block = new_block;
            }else{
                fat[prev_block] = new_block;                                               // we set the fat[prev_block] to the new block in order to link the current block to the new block
            }
            block = new_block;                                                             // Updating the block to the new block             
            fat[block] = -2;                                                               // we update the EOF by setting the new block to -2 in the FAT                  
        }

        char *block_data = (char*)(fs_start + CLUSTER_SIZE * (block));                     // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
        int write_size = CLUSTER_SIZE - byte_offset;                                       // Getting the write size (number of bytes to write to the block) by subtracting the byte offset from the cluster size
        if(write_size > rem_size){                                                         // If the write size is greater than the remaining size of the data, then we set the write size to the remaining size of the data
            write_size = rem_size;
        }

        memcpy(block_data + byte_offset, data + bytes_written, write_size);                // Copying the data to the block data starting from the byte offset
        bytes_written += write_size;                                                       // Updating the number of bytes written to the file                                  
        rem_size -= write_size;                                                            // Updating the remaining size of the data
        byte_offset = 0;                                                                   // Resetting the byte offset to 0 (since we are writing to a new block)                                  
        prev_block = block;
        block = fat[block];
        fh->pos += write_size;                                                             // Updating the current position in the file

        if(rem_size > 0 && fat[block] == -2){                                              // If the remaining size of the data is greater than 0 and the block is -2 in the FAT, then we need to allocate a new block in the FAT
            int new_block = free_fat_block();
            if(new_block == -1){
                handle_error_ret("\n#### ERROR! No free blocks in the FAT! ####\n", -1);
            }
            fat[prev_block] = new_block;
            block = new_block;
            fat[block] = -2;
        }
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
    int block_offset = data_pos / CLUSTER_SIZE;                                           // Getting the block offset (block number where the data is stored relative to the starting block of the file)
    int byte_offset = data_pos % CLUSTER_SIZE;                                            // Getting the byte offset (byte number in the block where the data is stored)
    int rem_size = buff_size;                                                             // Getting the remaining size of the buffer
    int bytes_read = 0;                                                                   // Number of bytes read from the file

    DirectoryElement *file = NULL;

    for(int i = 0; i < root_size; i++){                                                   // Searching for the file in the root
        if(strncmp(root[i].name, fh->directory->name, MAX_FILE_NAME) == 0){
            file = &root[i];
            break;
        }
    }

    if(!file){
        handle_error_ret("\n#### ERROR! File not found! ####\n", -1);
    }

    int block = file->start_block;                                                        // Getting the start block of the file in the FAT

    for(int i = 0; i < block_offset; i++){                                                // Traversing the FAT to the block offset (block number where the data is stored relative to the starting block of the file)
        if(fat[block] == -2){
            handle_error_ret("\n#### ERROR! Block offset is out of bounds! ####\n", -1);  // If the block is -2, then the block offset is out of bounds
        }
        block = fat[block];
    }                                                                                     // with this loop, we are moving through the FAT to the block where the fh->pos is pointing to


    while(rem_size > 0 && fat[block] != -2){
        char *block_data = (char*)(fs_start + CLUSTER_SIZE * (block));                    // Getting the block data (data stored in the block) by adding the block number to the starting address of the file system and multiplying it by the cluster size
        int read_size = CLUSTER_SIZE - byte_offset;                                       // Getting the read size (number of bytes to read from the block) by subtracting the byte offset from the cluster size
        if(read_size > rem_size){                                                         // If the read size is greater than the remaining size of the buffer, then we set the read size to the remaining size of the buffer
            read_size = rem_size;
        }

        memcpy(buff + bytes_read, block_data + byte_offset, read_size);                  // Copying the data from the block data starting from the byte offset to the buffer
        bytes_read += read_size;                                                         // Updating the number of bytes read from the file
        rem_size -= read_size;                                                           // Updating the remaining size of the buffer
        byte_offset = 0;                                                                 // Resetting the byte offset to 0 (since we are reading from a new block)
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

    for(int i = 0; i < root_size; i++){                                                   // Searching for the file in the root
        if(strncmp(root[i].name, fh->directory->name, MAX_FILE_NAME) == 0){
            file = &root[i];
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
    
    printf("\n#### LISTING DIRECTORY ####\n");
    for(int i = 0; i < root_size; i++){
        if(root[i].name[0] != '\0'){
            printf("File: %s\n", root[i].name);
        }
    }
    printf("\n");
    
    return 0;
}