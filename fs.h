#ifndef FS_H
#define FS_H

#define MAX_FILE_NAME 256             // Max file name length
#define MAX_FILE_SIZE 1024            // Max file size

typedef struct DirectoryElement {
    char name[MAX_FILE_NAME];         // File name
    int pos;                          // File position in the buffer  
} DirectoryElement;

typedef struct Directory{
    DirectoryElement* elements;       // Files in the directory
    int num_elements;                 // Number of files in the directory
} Directory;

typedef struct FileSystem{
    Directory curr_directory;         // Current directory
    char *buff;                       // Buffer to store files
    int buff_size;                    // Buffer size
} FileSystem;

typedef struct FileHandler{
    int pos;                          // Current position in the file
    int element_index;                // Index of the file in the directory
} FileHandler;


void init_fs(FileSystem *fs, int buff_size);                    // Initialize the file system

void create_file(FileSystem *fs, char *name);                   // Create a file
void erase_file(FileSystem *fs, char *name);                    // Erase a file
void write_file(FileSystem *fs, char *name, char *content);     // Write to a file (addition)
void read_file(FileSystem *fs, char *name, char *content);      // Read from a file
void seek_file(FileSystem *fs, char *name, int pos);            // Seek to a position in a file (fseek)
void create_directory(FileSystem *fs, char *name);              // Create a directory
void erase_directory(FileSystem *fs, char *name);               // Erase a directory
void change_directory(FileSystem *fs, char *name);              // Change the current directory
void list_directory(FileSystem *fs);                            // List the files in the current directory

#endif
