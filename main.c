#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);
    char command[256];

    while(1){
        printf("> ");
        if(fgets(command, 256, stdin) == NULL){
            break;
        }
        command[strcspn(command, "\n")] = 0;

        char* cmd = strtok(command, " ");
        if(cmd == NULL) continue;

        // function handlers here

    }

    

    return 0;
}