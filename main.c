#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);

    create_file(fs, "file1");
    create_file(fs, "file2");

    erase_file(fs, "file1");
    erase_file(fs, "file3");

    return 0;
}