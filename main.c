#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

#define FILE_IMAGE "fs.img"
#define SECTOR_SIZE 512
#define TOTAL_SECTORS 102400

void print_help() {
    printf("Available commands:\n");
    printf("  touch <name>              - Create a file\n");
    printf("  rm <name>                 - Erase a file\n");
    printf("  wof <name> <data>         - Write data to a file\n");
    printf("  cat <name>                - Read data from a file\n");
    printf("  fseek <name> <pos>        - Seek to a position in a file\n");
    printf("  mkdir <name>              - Create a directory\n");
    printf("  rmdir <name>              - Erase a directory\n");
    printf("  cd <name>                 - Change the current directory\n");
    printf("  fat <items>               - Prints the first number of <items> in the FAT table\n");
    printf("  ls                        - List the files in the current directory\n");
    printf("  help                      - Show this help message\n");
    printf("  exit                      - Exit the program\n");
}

int main() {
    char command[256];
    char name[MAX_FILE_NAME];
    char data[1024];
    int pos;
    char buff[1024];
    int items;
    FileHandler *fh;

    

    int fs = init_fs(FILE_IMAGE, TOTAL_SECTORS * SECTOR_SIZE);
    if(fs == -1){
        handle_error("Error initializing the file system.\n");
        free_fs();
        return -1;
    }
    printf("FAT File system initialized!\n");
    printf("Welcome to the FAT file system CLI.\n");
    print_help();

    while (1) {
        printf("fs> ");
        if (scanf("%s", command) != 1) {
            continue;
        }

        if (strcmp(command, "touch") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid file name.\n");
                continue;
            }
            fh = create_file(name);
            if (fh == NULL) {
                handle_error("Error creating the file.\n");
            } else {
                printf("File '%s' created.\n", name);
            }
        } else if (strcmp(command, "rm") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid file name.\n");
                continue;
            }
            if (erase_file(name) == -1) {
                handle_error("Error erasing the file.\n");
            } else {
                printf("File '%s' erased.\n", name);
            }
        } else if (strcmp(command, "wof") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid file name.\n");
                continue;
            }
            getchar();
            if (fgets(data, sizeof(data), stdin) == NULL) {
                printf("Invalid input.\n");
                continue;
            }
            data[strcspn(data, "\n")] = '\0';
            fh = get_file_handler(name);
            if (fh == NULL) {
                handle_error("Error file not found!\n");
                continue;
            }
            if (write_file(fh, data) == -1) {
                handle_error("Error writing to the file.\n");
            } else {
                printf("Data written to '%s'.\n", name);
            }
        } else if (strcmp(command, "cat") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid input.\n");
                continue;
            }
            fh = get_file_handler(name);
            if (fh == NULL) {
                handle_error("Error opening the file.\n");
                continue;
            }
            if (read_file(fh, buff, 100) == -1) {
                handle_error("Error reading from the file.\n");
            } else {
                printf("Read from file '%s': %s\n", name, buff);
            }
        } else if (strcmp(command, "fseek") == 0) {
            if (scanf("%s", name) != 1 || scanf("%d", &pos) != 1) {
                printf("Invalid input.\n");
                continue;
            }
            fh = get_file_handler(name);
            if (fh == NULL) {
                handle_error("Error opening the file.\n");
                continue;
            }
            if (seek_file(fh, pos) == -1) {
                handle_error("Error seeking to the position in the file.\n");
            } else {
                printf("File '%s' seeked to position %d.\n", name, pos);
            }
        } else if (strcmp(command, "mkdir") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid directory name.\n");
                continue;
            }
            if (create_directory(name) == -1) {
                handle_error("Error creating the directory.\n");
            } else {
                printf("Directory '%s' created.\n", name);
            }
        } else if (strcmp(command, "rmdir") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid directory name.\n");
                continue;
            }
            if (erase_directory(name) == -1) {
                handle_error("Error erasing the directory.\n");
            } else {
                printf("Directory '%s' erased.\n", name);
            }
        } else if (strcmp(command, "cd") == 0) {
            if (scanf("%s", name) != 1) {
                printf("Invalid directory name.\n");
                continue;
            }
            if (change_directory(name) == -1) {
                handle_error("Error changing the directory.\n");
            } else {
                printf("Changed to directory '%s'.\n", name);
            }
        } else if (strcmp(command, "fat") == 0) {
            if (scanf("%d", &items) != 1) {
                printf("Invalid number of items.\n");
                continue;
            }
            print_fat(items);
        } else if (strcmp(command, "ls") == 0) {
            list_directory();
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    free_fs();
    return 0;
}
