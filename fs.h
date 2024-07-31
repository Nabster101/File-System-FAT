#ifndef FS_H
#define FS_H

#define MAX_FILE_NAME 30             // Max file name length
#define CLUSTER_SIZE 4096              // Cluster size
#define FAT_ELEMENTS CLUSTER_SIZE/4    // Number of elements in the FAT

#define SECTOR_SIZE 512
#define TOTAL_SECTORS 102400
#define SIZE (SECTOR_SIZE*TOTAL_SECTORS)

#define handle_error(msg) do { printf(msg); } while (0)
#define handle_error_ret(msg, ret) do { printf(msg); return ret; } while (0)

typedef struct FileHandler FileHandler;

typedef struct {
    int bytes_per_block;
    int fat_entries;
    int cluster_size;
    int fat_size;
    int data_size;
    int total_blocks;
    char current_directory[25];
} FileSystem;

typedef struct DirectoryElement{                                // File structure
    char name[MAX_FILE_NAME];                                   // File name     
    int start_block;                                            // Start block in the FAT         
    int size;                                                   // File size
    int entry_num;
    int is_directory;                                           // Is a directory
    struct DirectoryElement *parent;                           // Parent directory
} DirectoryElement;

struct FileHandler{
    int pos;                                                    // Current position in the file
    DirectoryElement *file_entry;                                // Pointer to the directory containing the file
};

int init_fs(const char* fileImage);                   // Initialize the file system
int create_file(const char *name, int size, const char* data); // Create a file
int erase_file(const char *name);                               // Erase a file
int write_file(const char* name, const char* data);
int read_file(FileHandler *fh, char *buff, int buff_size);      // Read from a file
int seek_file(FileHandler *fh, int pos);                        // Seek to a position in a file (fseek)
int create_directory(const char *name);                         // Create a directory
int erase_directory(const char *name);                          // Erase a directory
int change_directory(const char *name);                         // Change the current directory
int list_directory();                                           // List the files in the current directory    
void free_fs();                                                 // Free the file system       
void print_fat(int items);                                      // Print the FAT   
FileHandler *get_file_handler(const char *name);                // Get the file handler of a file

int load_fs(const char *filename);
int save_fs();

#endif 
