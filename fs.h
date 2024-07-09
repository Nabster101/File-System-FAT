#ifndef FS_H
#define FS_H

#define MAX_FILE_NAME 30             // Max file name length
#define CLUSTER_SIZE 4096              // Cluster size
#define FAT_ELEMENTS CLUSTER_SIZE/4    // Number of elements in the FAT

#define handle_error(msg) do { printf(msg); } while (0)
#define handle_error_ret(msg, ret) do { printf(msg); return ret; } while (0)

typedef struct FileHandler FileHandler;

typedef struct DirectoryElement{                                // File structure
    char name[MAX_FILE_NAME];                                   // File name     
    int current_block;                                          // Current block in the FAT
    int start_block;                                            // Start block in the FAT         
    int size;                                                   // File size
    int is_directory;                                           // Is a directory
    struct DirectoryElement *parent;                           // Parent directory
    FileHandler *fh;                                            // File handler
} DirectoryElement;

struct FileHandler{
    char file_name[MAX_FILE_NAME];                                   // File name
    int size;                                                   // File size
    int pos;                                                    // Current position in the file
    DirectoryElement *directory;                                // Pointer to the directory containing the file
};

int init_fs(const char* fileImage, int size);                   // Initialize the file system
FileHandler* create_file(const char *name);                     // Create a file
int erase_file(const char *name);                               // Erase a file
int write_file(FileHandler *fh, const char *data);              // Write to a file (addition)
int read_file(FileHandler *fh, char *buff, int buff_size);      // Read from a file
int seek_file(FileHandler *fh, int pos);                        // Seek to a position in a file (fseek)
int create_directory(const char *name);                         // Create a directory
int erase_directory(const char *name);                          // Erase a directory
int change_directory(const char *name);                         // Change the current directory
int list_directory();                                           // List the files in the current directory    
void free_fs();                                                 // Free the file system       
void print_fat(int items);                                      // Print the FAT   
FileHandler *get_file_handler(const char *name);                // Get the file handler of a file

#endif 
