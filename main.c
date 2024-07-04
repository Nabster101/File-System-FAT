#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);

    create_directory(fs, "dir1");
    create_directory(fs, "dir2");

    FileHandler* fh = create_file(fs, "file1");
    list_directory(fs);

    FileHandler *fh2 = create_file(fs, "file2");
    list_directory(fs);

    change_directory(fs, "dir1");
    list_directory(fs);

    FileHandler *fh3 = create_file(fs, "file3");
    list_directory(fs);

    write_file(fs, fh, "Hello, World! This is file1.");
    read_file(fs, fh);
    seek_file(fh, 7);
    read_file(fs, fh);

    erase_file(fs, "file1");
    list_directory(fs);
    change_directory(fs, "..");
    erase_file(fs, "file1");
    list_directory(fs);

    erase_directory(fs, "dir2");
    list_directory(fs);
    erase_directory(fs, "dir1");
    list_directory(fs);

    

    return 0;
}