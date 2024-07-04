#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);
    FileSystem* fs2 = NULL;

    FileHandler* fh = create_file(fs, "file1");
    FileHandler* fh2 = NULL;

    write_file(fs, fh, "Hello World!");
    // write_file(fs2, fh2, "Ao");
    // write_file(fs, fh2, "Ao");

    read_file(fs, fh);

    seek_file(fh, 2);

    read_file(fs, fh);

    seek_file(fh, 0);

    read_file(fs, fh);
    




    return 0;
}