#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);
    FileHandler* fh = create_file(fs, "file1");

    create_file(fs, "file2");

    erase_file(fs, "file1");
    erase_file(fs, "file3");

    write_file(fs, "file5", "Hello, World!");
    write_file(fs, "file2", "Hello, World!");

    read_file(fs, "file2");
    read_file(fs, "file3");


    return 0;
}