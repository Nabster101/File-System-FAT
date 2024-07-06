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