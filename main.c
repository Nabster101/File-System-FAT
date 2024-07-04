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

        if(strcmp(cmd, "touch") == 0){
            const char* file_name = strtok(NULL, " ");
            if(file_name) handle_create_file(fs, file_name);
            else printf("Usage: touch <file_name>\n");
        }

        if(strcmp(cmd, "rm") == 0){
            const char* file_name = strtok(NULL, " ");
            if(file_name) handle_erase_file(fs, file_name);
            else printf("Usage: rm <file_name>\n");
        }

        if(strcmp(cmd, "wof") == 0){
            const char* file_name = strtok(NULL, " ");
            const char* data = strtok(NULL, "");
            if(file_name && data) handle_write_file(fs, file_name, data);
            else printf("Usage: wof <file_name> <data>\n");
        }

        if(strcmp(cmd, "cat") == 0){
            const char* file_name = strtok(NULL, " ");
            if(file_name) handle_read_file(fs, file_name);
            else printf("Usage: cat <file_name>\n");
        }

        
    }

    

    return 0;
}