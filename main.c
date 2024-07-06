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

    if(write_file(fh, "Hello World! ") == -1){
        handle_error("Error writing to the file.\n");
        return -1;
    }

    if(write_file(fh, "This is a test for the write file!") == -1){
        handle_error("Error writing to the file.\n");
        return -1;
    }

    if(seek_file(fh, 6) == -1){
        handle_error("Error seeking to the beginning of the file.\n");
        return -1;
    }

    char buff[100] = {0};
    if(read_file(fh, buff, sizeof(buff)) == -1){
        handle_error("Error reading from the file.\n");
        return -1;
    }

    printf("File content: %s\n", buff);

    


    return 0;
}