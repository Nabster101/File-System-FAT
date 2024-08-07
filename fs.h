#ifndef FS_H
#define FS_H

#define MAX_FILE_NAME 256             // Max file name length
#define MAX_FILE_SIZE 1024            // Max file size
#define MAX_SUBDR 256    // Max number of subdirectories

#define handle_error(msg) do { printf(msg); } while (0)
#define handle_error_ret(msg, ret) do { printf(msg); return ret; } while (0)

typedef struct Directory Directory;

typedef struct DirectoryElement{                // File structure
    char name[MAX_FILE_NAME];                   // File name     
    int pos;                                    // File position in the buffer          
    Directory *parent;                          // Pointer to the parent directory
} DirectoryElement;

struct Directory{                               // Directory structure     
    Directory *parent;                          // Pointer to the parent directory
    Directory *subdirectories[MAX_SUBDR];       // Pointer to the subdirectories
    DirectoryElement *elements;                 // Pointer to the files in the directory
    char dir_name[MAX_FILE_NAME];               // Directory name
    int num_elements;                           // Number of files in the directory
    int num_subdirectories;                     // Number of subdirectories in the directory
};

typedef struct FileSystem{                      // File System structure
    char *buff;                                 // Buffer to store files
    int buff_size;                              // Buffer size
    Directory *root_directory;                  // Pointer to the root directory
    Directory *curr_directory;                  // Pointer to the current directory
} FileSystem;

typedef struct FileHandler{
    int pos;                                    // Current position in the file
    int element_index;                          // Index of the file in the directory
    Directory *directory;                       // Pointer to the directory containing the file
} FileHandler;

FileSystem* init_fs(int buff_size);                                     // Initialize the file system
FileHandler* create_file(FileSystem *fs, const char *name);             // Create a file
void erase_file(FileSystem *fs, const char *name);                      // Erase a file
void write_file(FileSystem *fs, FileHandler *fh, const char *data);     // Write to a file (addition)
void read_file(FileSystem *fs, FileHandler *fh);                        // Read from a file
void seek_file(FileHandler *fh, int pos);                               // Seek to a position in a file (fseek)
void create_directory(FileSystem *fs, const char *name);                // Create a directory
void erase_directory(FileSystem *fs, const char *name);                 // Erase a directory
void change_directory(FileSystem *fs, const char *name);                // Change the current directory
void list_directory(FileSystem *fs);                                    // List the files in the current directory              

#endif 
