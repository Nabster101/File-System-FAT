#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main(){
    FileSystem* fs = init_fs(1024);
    char command[256];                                                  // This would be the command buffer

    while(1){
        printf("> ");                                                   // Print prompt                                    
        if(fgets(command, 256, stdin) == NULL){                         // Read command from stdin
            break;
        }
        command[strcspn(command, "\n")] = 0;                            // we remove the newline character from the command to avoid issues with strtok

        char* cmd = strtok(command, " ");                               // we split the command into tokens using the space character as delimiter           
        if(cmd == NULL) continue;

        if(strcmp(cmd, "touch") == 0){                                  // touch -> create_file          
            const char* file_name = strtok(NULL, " ");                  // we get the file name from the command using strtok and the space character as delimiter. We use NULL as the first argument to continue from the last position in the string from the previous call to strtok
            if(file_name) handle_create_file(fs, file_name);            // if the file name is not NULL we call the handle_create_file function
            else printf("Usage: touch <file_name>\n");                  // if the file name is NULL we print a usage message
        }

        else if(strcmp(cmd, "rm") == 0){                                // rm -> erase_file
            const char* file_name = strtok(NULL, " ");
            if(file_name) handle_erase_file(fs, file_name);
            else printf("Usage: rm <file_name>\n");
        }

        else if(strcmp(cmd, "wof") == 0){                               // wof -> write_file               
            const char* file_name = strtok(NULL, " ");
            const char* data = strtok(NULL, "");
            if(file_name && data) handle_write_file(fs, file_name, data);
            else printf("Usage: wof <file_name> <data>\n");
        }

        else if(strcmp(cmd, "cat") == 0){                               // cat -> read_file                
            const char* file_name = strtok(NULL, " ");
            if(file_name) handle_read_file(fs, file_name);
            else printf("Usage: cat <file_name>\n");
        }

        else if(strcmp(cmd, "mkdir") == 0){                            // mkdir -> create_directory                     
            const char* dir_name = strtok(NULL, " ");
            if(dir_name) handle_create_directory(fs, dir_name);
            else printf("Usage: mkdir <dir_name>\n");
        }

        else if(strcmp(cmd, "cd") == 0){                               // cd -> change_directory                                                   
            const char* dir_name = strtok(NULL, " ");
            if(dir_name) handle_change_directory(fs, dir_name);
            else printf("Usage: cd <dir_name>\n");
        }

        else if(strcmp(cmd, "ls") == 0){                               // ls -> list_directory                                  
            handle_list_directory(fs);
        }

        else if(strcmp(cmd, "rmdir") == 0){                            // rmdir -> erase_directory                                           
            const char* dir_name = strtok(NULL, " ");
            if(dir_name) handle_erase_directory(fs, dir_name);
            else printf("Usage: rmdir <dir_name>\n");
        }

        else if(strcmp(cmd, "exit") == 0){                             // exit -> exit the program                                            
            break;
        }

        else if(strcmp(cmd, "help") == 0){
            printf("-> touch <file_name> - create a file\n");
            printf("-> rm <file_name> - erase a file\n");
            printf("-> wof <file_name> <data> - write to a file\n");
            printf("-> cat <file_name> - read from a file\n");
            printf("-> mkdir <dir_name> - create a directory\n");
            printf("-> cd <dir_name> - change the current directory\n");
            printf("-> ls - list the files in the current directory\n");
            printf("-> rmdir <dir_name> - erase a directory\n");
            printf("-> exit - exit the program\n");
        }
    
        else{
            printf("Unknown command\n");
        }
                
    }

    return 0;
}