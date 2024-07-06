#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

#define FILE_IMAGE "fs.img"
#define SECTOR_SIZE 512
#define TOTAL_SECTORS 102400

int main(){
    
    int fs = init_fs(FILE_IMAGE, TOTAL_SECTORS * SECTOR_SIZE);
    if(fs == -1){
        handle_error("Error initializing the file system.\n");
        return -1;
    }
    printf("FAT File system initialized!\n");

    FileHandler *fh = create_file("file1.txt");
    if(fh == NULL){
        handle_error("Error creating the file.\n");
        return -1;
    }

    FileHandler *fh2 = create_file("file2.txt");
    if (fh == NULL)    {
        handle_error("Error creating the file.\n");
        return -1;
    }

    list_directory();

    erase_file("file1.txt");

    list_directory();

    erase_file("file2.txt");

    list_directory();


    return 0;
}