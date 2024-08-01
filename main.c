#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "fs.h"

#define FILE_IMAGE "DATA_FILE.dat"


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
    char* TEST_4100 = "SFEFWLorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi non sapien sollicitudin, aliquet felis eu, sagittis est. Mauris ipsum tortor, malesuada sit amet tempus eget, tempor et eros. Nulla dapibus nisi felis, vitae condimentum est eleifend sit amet. Etiam nec mi sed justo semper viverra. Morbi ante tellus, sagittis eu nisl vitae, porttitor ultrices ipsum. Sed quis viverra lorem, vitae consequat mi. Ut efficitur, metus at vestibulum sodales, lectus quam porttitor sem, eget viverra dolor urna sed sem. Integer ac convallis neque, in efficitur elit. Phasellus blandit suscipit neque. In sed turpis eu diam malesuada tincidunt. Duis eget leo aliquet, dapibus dui vel, condimentum libero. Nam vel eros tempor, ultricies metus in, consequat eros. Duis vitae blandit lectus. Etiam feugiat leo quis turpis aliquet, vitae semper elit iaculis. Aliquam cursus, velit vel elementum consectetur, nulla justo consectetur sapien, non ornare ipsum ipsum at eros. Pellentesque hendrerit et turpis eu ullamcorper. Vestibulum in vestibulum quam. Pellentesque dui nisl, scelerisque id placerat sit amet, interdum at tellus. Nulla ut vestibulum risus. Aenean in urna dapibus, efficitur augue nec, feugiat massa. Vestibulum at sem sit amet arcu varius euismod. Nam vitae ipsum ut odio porta blandit. Vestibulum egestas neque non sagittis accumsan. Morbi aliquet consectetur auctor. Vivamus porttitor, augue a ultricies fringilla, orci leo faucibus mi, a accumsan turpis libero et nulla. Mauris tristique ultricies est, eu sollicitudin enim vestibulum facilisis. Donec ullamcorper eros non tincidunt pellentesque. Aliquam in lobortis libero. Duis sodales turpis dictum placerat pharetra. Donec maximus dolor ut libero mollis eleifend. Integer imperdiet, libero vel egestas tincidunt, urna nisl eleifend mi, sed dapibus justo nisi et sapien. Phasellus sollicitudin urna sit amet urna ornare, ac placerat metus ultrices. Curabitur nunc nunc, pharetra quis dui et, dignissim finibus orci. Sed in est odio. Aliquam finibus vehicula metus at pretium. Aliquam luctus velit sed iaculis laoreet. In varius erat ut leo tempus placerat. Vestibulum vel enim a sapien convallis faucibus ac vel lorem. In tincidunt, nisi et consequat finibus, quam nulla ullamcorper libero, in rhoncus elit quam quis odio. Quisque massa orci, ullamcorper eget accumsan et, volutpat non sapien. Nullam ultricies sit amet dui ac imperdiet. Integer quis purus dolor. Phasellus sem erat, eleifend ut semper congue, porta eget lorem. Donec commodo lacus ac metus hendrerit, id placerat sapien ultrices. Mauris auctor augue dapibus magna sagittis, vel fermentum odio fermentum. Ut sem turpis, hendrerit quis lectus non, luctus venenatis sapien. Mauris eu nibh sit amet tellus malesuada ultrices vel at ligula. Quisque vulputate purus id blandit porttitor. Praesent sed mi vitae diam eleifend venenatis. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam ultrices vitae ligula a dictum. Donec iaculis, quam quis elementum placerat, ligula lorem molestie diam, sed mollis metus massa et orci. Cras lobortis velit tincidunt, maximus urna id, facilisis quam. Curabitur euismod lectus cursus, ornare ante a, lacinia ligula. Sed non pretium arcu. Fusce sed orci eget lacus ultricies faucibus. Maecenas mattis dolor in ultricies cursus. Integer in erat vel turpis egestas pellentesque varius vel metus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Vivamus nec felis ullamcorper, luctus ligula non, facilisis enim. Aliquam et bibendum nisl, egestas mollis ante. Donec et velit convallis, ultricies est non, finibus massa. Nam eget sollicitudin metus, vitae vestibulum justo. Etiam suscipit dui vitae blandit eleifend. Nam luctus risus varius, porttitor quam nec, egestas orci. Morbi hendrerit arcu in sollicitudin scelerisque. Nunc tempus, ipsum consequat dapibus consequat, tortor nibh finibus diam, nec aliquet odio risus sed dolor. Nullam hendrerit elementum gravida. Donec velit ligula, aliquam convallis sodales tristique, porttitor aliquam orci. Suspendisse bibendum lectus risus. Nunc vulputate at.";
    char* TEST_7030 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed ornare quam sapien. In elit nunc, tincidunt mollis maximus eget, facilisis sed odio. Quisque eu tellus gravida, congue mauris eu, consectetur arcu. Ut at lectus convallis, aliquam justo at, ultrices diam. Nunc at pellentesque diam. Ut rutrum convallis metus, in faucibus dolor tempus ac. Proin quis consectetur neque. Integer efficitur justo est, ut sagittis arcu gravida sit amet. Vestibulum eu elit erat. Donec sed purus convallis, ultrices massa eget, mollis erat. Maecenas luctus enim tempus elit eleifend dapibus. Quisque at molestie quam, ut mattis erat. Nulla id blandit lorem. Donec placerat ipsum ac augue maximus accumsan. Maecenas laoreet diam nec massa congue, euismod aliquet est malesuada. Nunc vitae metus at mi feugiat suscipit et sit amet nisl. Curabitur at placerat sapien. Phasellus vestibulum sagittis dictum. Etiam id iaculis ipsum. Nam sapien lectus, gravida id orci id, sagittis posuere lectus. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed ut quam id nibh ultrices laoreet eget sit amet arcu. Ut nec sem malesuada, auctor ipsum non, molestie risus. Mauris vitae ligula eleifend, finibus quam quis, venenatis ex. Nunc nisl diam, euismod vitae neque et, commodo bibendum diam. Aliquam faucibus facilisis tempor. Vestibulum sed semper risus. Suspendisse massa felis, tincidunt nec placerat et, vehicula vitae risus. Etiam malesuada orci in elit aliquam, at pretium ante tincidunt. Quisque sagittis iaculis est sed mollis. Nam vitae libero tempus, interdum tortor quis, pretium velit. Sed mollis, tellus eu lacinia placerat, ante tellus facilisis quam, gravida accumsan est risus in augue. Donec egestas dapibus sollicitudin. In ac rhoncus quam. In rutrum, turpis non rutrum consequat, diam ante varius purus, a egestas lorem metus ac urna. Donec et convallis lacus. Donec ut gravida diam, sed rutrum sem. In hac habitasse platea dictumst. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Pellentesque aliquam, nunc sit amet tincidunt suscipit, turpis metus scelerisque nibh, sed lacinia neque ante et risus. Nullam pellentesque laoreet est eu luctus. Proin ac nulla dui. Suspendisse posuere nunc a sagittis varius. Phasellus at eros ac arcu iaculis fermentum bibendum non neque. Integer sed feugiat nisl. Maecenas sit amet ipsum suscipit augue ullamcorper porttitor at non risus. Sed ultrices felis quam, sit amet finibus diam convallis vel. Maecenas leo mi, pellentesque id mi in, accumsan tempus mi. Integer pulvinar libero augue, a commodo justo placerat a. Sed tincidunt iaculis tempor. Suspendisse nec risus cursus, venenatis sapien ut, facilisis risus. Suspendisse sodales tristique purus, id vestibulum orci accumsan sed. Aenean rutrum elementum turpis, vel consequat enim efficitur a. Cras maximus finibus enim ac posuere. Fusce scelerisque arcu vitae libero condimentum mollis. Morbi pellentesque diam et blandit convallis. Nullam sapien tellus, vehicula at velit eu, consequat consectetur diam. Nullam vitae ultrices metus. Duis turpis tortor, ultrices luctus convallis vitae, gravida id est. Integer porta nibh ex, non posuere dolor commodo in. Ut urna nibh, condimentum et finibus vitae, efficitur id libero. Praesent lacus ligula, vulputate eget tempor ac, venenatis bibendum sem. Aenean a turpis nisi. Pellentesque a orci at elit accumsan egestas eu vel est. In cursus sit amet neque nec rhoncus. Aenean feugiat ultricies tortor, et dictum lacus convallis ut. Suspendisse potenti. Ut in tincidunt nulla. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse accumsan mattis efficitur. Nam vehicula erat malesuada auctor varius. Aliquam ac porttitor est, at interdum odio. Aenean mauris nisi, semper eget lectus sed, scelerisque congue purus. Integer auctor ipsum at ipsum tincidunt, mollis commodo turpis commodo. Phasellus venenatis ultrices enim ac rhoncus. Nullam sodales odio vel nibh bibendum, ullamcorper venenatis tortor auctor. Etiam faucibus lectus sed sagittis imperdiet. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nam turpis massa, tincidunt dictum lobortis nec, lobortis vel odio. Nulla sodales elit urna, sit amet consequat neque venenatis vehicula. Nullam felis nibh, dictum sit amet mi a, sodales porta dolor. Morbi ut justo semper sapien euismod hendrerit. Fusce a auctor nibh, sed ultrices ex. Vivamus at ex ac augue vestibulum placerat. Sed imperdiet nunc enim, eu semper nisi mollis nec. Integer diam turpis, posuere eu orci in, viverra consectetur turpis. Phasellus blandit dolor elit, at interdum nunc luctus at. Nam posuere volutpat arcu, sed consequat tortor interdum nec. Quisque lacinia venenatis libero pellentesque dapibus. Donec mattis eu augue eu molestie. Aliquam facilisis dui in dolor commodo, rutrum maximus ex imperdiet. Pellentesque sit amet libero ac nibh ullamcorper tristique. Duis purus neque, tempor ac nulla sit amet, ullamcorper interdum felis. Duis nunc magna, porta a nibh vel, lacinia vehicula nulla. Aliquam dignissim feugiat efficitur. Integer non tortor mattis, malesuada nulla et, blandit nunc. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Sed quis ipsum quis neque commodo maximus quis ac purus. Ut a tellus vel sem scelerisque cursus. Morbi vitae posuere augue. Pellentesque lobortis metus id ornare tempus. Aenean pulvinar molestie eros, id aliquam urna malesuada quis. Nunc quis consequat tellus. Etiam id lorem convallis, tincidunt quam nec, fermentum nunc. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras feugiat hendrerit ex vitae pellentesque. Ut arcu felis, congue in gravida quis, tristique non sem. Vivamus finibus eu est vel placerat. Donec eu eros tincidunt, tristique orci sed, scelerisque erat. Morbi et purus in risus maximus maximus. Duis turpis ipsum, ornare in turpis hendrerit, gravida eleifend elit. Donec pellentesque neque vel nibh pulvinar efficitur. Aliquam erat volutpat. Etiam odio dui, euismod sit amet tellus et, auctor cursus eros. Pellentesque vestibulum libero vitae nulla viverra imperdiet. Nullam consequat arcu lectus, ac laoreet ex sodales in. Etiam quam nunc, aliquam et laoreet eu, tempor non massa. Etiam gravida vel libero ac iaculis. Morbi et nisl odio. Nulla vel tristique urna. Sed non lorem nec arcu laoreet aliquet sed nec sem. Quisque at erat scelerisque massa molestie luctus eu vel lorem. Fusce vel varius metus. Phasellus fringilla, ante sit amet auctor tincidunt, orci elit sollicitudin odio, eu varius risus mauris in eros. Duis hendrerit elit in elit viverra feugiat sed eget lectus. In hac habitasse platea dictumst. Nullam in dignissim odio, eu venenatis enim. Nam vitae finibus nisl, eu ultricies ante. Donec tristique lectus a massa tincidunt lobortis. Ut elit ex, tincidunt ut pharetra vitae, pharetra et eros. Maecenas id iaculis est. Integer condimentum, quam in fringilla scelerisque, leo odio finibus est, et lacinia nisl lacus dictum nulla. Donec libero.";
    char* TEST_CODE2= "#include \"  fs.h\"   #include stdio.h> #include <stdlib.h> #include <string.h> #include <stdint.h> #include <unistd.h> #include <fcntl.h> #include <sys/mman.h> int *fat = NULL; DirectoryElement *root = NULL; DirectoryElement *current_directory = NULL; DirectoryElement *parent_directory = NULL; void *fs_start = NULL; int root_size = 0; int fat_size = 0; int num_directories = 0; int init_fs(const char* fileImage, int size){ if(!fileImage){ handle_error_ret(\"   #### ERROR! File image not found! #### \"  , -1); } if(size <= 0){ handle_error_ret(\"   #### ERROR! Size is less than or equal to 0! #### \"  , -1); } int fd = open(fileImage, O_RDWR | O_CREAT, 0666); if(fd == -1){ handle_error_ret(\"   #### ERROR! Couldn't open the fileImage! #### \"  , -1); } if(ftruncate(fd, size) == -1){ free_fs(); handle_error_ret(\"   #### ERROR! Couldn't truncate the fileImage! #### \"  , -1); } fs_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); if(fs_start == MAP_FAILED){ free_fs(); handle_error_ret(\"   #### ERROR! Couldn't map the fileImage! #### \"  , -1); } fat = fs_start; root = (DirectoryElement*) (fs_start + CLUSTER_SIZE); memset(fat, 0, FAT_ELEMENTS * 4); printf(\"  FAT initialized! \"  ); memset(root, 0, (size - CLUSTER_SIZE)); printf(\"  Root initialized! \"  ); root_size = sizeof(DirectoryElement); fat_size = FAT_ELEMENTS; current_directory = root; current_directory->size = root_size; current_directory->is_directory = 1; current_directory->start_block = 0; fat[current_directory->start_block] = -1; strncpy(current_directory->name, \"  ROOT\"  , MAX_FILE_NAME); parent_directory = current_directory; num_directories++; return 0; } void print_fat(int items){ if(items > fat_size){ items = fat_size; } for(int i = 0; i < items; i++){ printf(\"  FAT[%d]: %d \"  , i, fat[i]); } } int free_fat_block(){ for(int i = 2; i < fat_size; i++){ if(fat[i] == 0){ return i; } } return -1; } FileHandler* create_file(const char *name){ if(!name){ handle_error_ret(\"   #### ERROR! File name not found! #### \"  , NULL); } if(strlen(name) > MAX_FILE_NAME){ handle_error_ret(\"   #### ERROR! File name is too long! #### \"  , NULL); } for(int i = 0; i < current_directory->size; i++){ if(strcmp(current_directory[i].name, name) == 0){ handle_error_ret(\"   #### ERROR! File with this name already exists! #### \"  , NULL); } } int free_block = free_fat_block(); if(free_block == -1){ handle_error_ret(\"   #### ERROR! No free blocks in the FAT! #### \"  , NULL); } FileHandler *fh = (FileHandler*)malloc(sizeof(FileHandler)); if(!fh){ handle_error_ret(\"   #### ERROR! Couldn't allocate memory for the file handler! #### \"  , NULL); } fh->pos = 0; fh->directory = current_directory; strncpy(fh->file_name, name, MAX_FILE_NAME-1); fh->file_name[MAX_FILE_NAME-1] = '0'; for(int i = 0; i < current_directory->size; i++){ if(current_directory[i].size == 0 && current_directory[i].name[0] == '0' && current_directory[i].is_directory == 0){ strncpy(current_directory[i].name, name, MAX_FILE_NAME-2); current_directory[i].name[MAX_FILE_NAME-1] = '0'; current_directory[i].start_block = free_block; fat[free_block] = -2; current_directory->size = sizeof(DirectoryElement)*i; current_directory[i].is_directory = 0; current_directory[i].fh = fh; break; } } return fh; } int erase_file(const char *name){ if(!name){ handle_error_ret(\"   #### ERROR! File name not found! #### \"  , -1); } for(int i = 1; i < current_directory->size; i++){ if(strcmp(current_directory[i].name, name) == 0){ int block = current_directory[i].start_block; printf(\"  Erasing file: %s \"  , name); while(fat[block] != 0){ if(fat[block] == -2){ fat[block] = 0; printf(\"  Freed block: %u \"  , block); break; } int next_block = fat[block]; fat[block] = 0; printf(\"  Freed block: %u \"  , block); block = next_block; } memset(&current_directory[i], 0, sizeof(DirectoryElement)); printf(\"  File %s erased successfully! \"  , name); return 0; } } return -1; } int write_file(FileHandler *fh, const char *data){ if(!fh){ handle_error_ret(\"   #### ERROR! File handler not found! #### \"  , -1); } if(!data){ handle_error_ret(\"   #### ERROR! Data not found! #### \"  , -1); } int data_size = strlen(data); int data_pos = fh->pos; int rem_size = data_size; int bytes_written = 0; DirectoryElement *file = NULL; int is_in_current_directory = 0; if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){ is_in_current_directory = 1; } if(is_in_current_directory){ for(int i = 0; i < current_directory->size; i++){ if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){ file = &current_directory[i]; file->current_block = file->start_block; break; } } }else{ handle_error_ret(\"   #### ERROR! File not found in the current directory! #### \"  , -1); } int block = file->start_block; while(rem_size > 0){ if(fat[block] == -2){ int new_block = free_fat_block(); if(new_block == -1){ handle_error_ret(\"   #### ERROR! No free blocks in the FAT! #### \"  , -1); } fat[block] = new_block; file->current_block = block; fat[new_block] = -2; } char *block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block)); int write_size = CLUSTER_SIZE - data_pos; if(write_size > rem_size){ write_size = rem_size; } memcpy(block_data + data_pos, data + bytes_written, write_size); bytes_written += write_size; rem_size -= write_size; data_pos = 0; fh->pos += write_size; if(rem_size > 0){ int new_block = free_fat_block(); if(new_block == -1){ handle_error_ret(\"   #### ERROR! No free blocks in the FAT! #### \"  , -1); } fat[file->current_block] = new_block; file->current_block = new_block; fat[new_block] = -2; } } file->size += bytes_written; return bytes_written; } int read_file(FileHandler *fh, char*buff, int buff_size){ if(!fh){ handle_error_ret(\"   #### ERROR! File handler not found! #### \"  , -1); } if(!buff){ handle_error_ret(\"   #### ERROR! Buffer not found! #### \"  , -1); } int data_pos = fh->pos; int rem_size = buff_size; int bytes_read = 0; DirectoryElement *file = NULL; int is_in_current_directory = 0; if(strncmp(current_directory->name, fh->directory->name, MAX_FILE_NAME) == 0){ is_in_current_directory = 1; } if(is_in_current_directory){ for(int i = 0; i < current_directory->size; i++){ if(strncmp(current_directory[i].name, fh->file_name, MAX_FILE_NAME) == 0 && current_directory[i].is_directory == 0){ file = &current_directory[i]; file->current_block = file->start_block; break; } } }else{ handle_error_ret(\"   #### ERROR! File not found in the current directory! #### \"  , -1); } if(!file){ handle_error_ret(\"   #### ERROR! File not found! #### \"  , -1); } int block = file->start_block; while(rem_size > 0 && fat[block] != -2){ char *block_data = (char*)(fs_start + CLUSTER_SIZE * (file->current_block)); int read_size = CLUSTER_SIZE - data_pos; if(read_size > rem_size){ read_size = rem_size; } memcpy(buff + bytes_read, block_data + data_pos, read_size); bytes_read += read_size; rem_size -= read_size; data_pos = 0; fh->pos += read_size; } if (bytes_read < buff_size){ buff[bytes_read] = '0'; } return bytes_read; } int seek_file(FileHandler *fh, int p";
    char command[256];
    char name[MAX_FILE_NAME];
    char data[1024000];
    int pos;
    char buff[150000];
    int items;
    FileHandler *fh;

    printf("Welcome to the FAT file system CLI.\n");
    print_help();

    while (1) {
        printf("fs> ");
        if (scanf("%s", command) != 1) {
            continue;
        }

        if (strcmp(command, "touch") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid file name.\n");
                continue;
            }
            if (create_file(name, 0, "") == -1) {
                handle_error("\n#### ERROR! Cannot create file! ####\n");
            } else {
                printf("- File '%s' created.\n", name);
            }

        } else if(strcmp(command, "mkfs") == 0){
            printf("- Initializing the FAT file system...\n");
            init_fs(FILE_IMAGE);
            printf("- FAT file system initialized.\n");
        
        } else if(strcmp(command, "loadfs") == 0){
            printf("- Loading the FAT file system...\n");
            load_fs(FILE_IMAGE);
            printf("- FAT file system loaded.\n");
        
        } else if(strcmp(command, "savefs") == 0){
            printf("- Saving the FAT file system...\n");
            save_fs();
            printf("- FAT file system saved.\n");

        } else if (strcmp(command, "rm") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid file name.\n");
                continue;
            }
            if (erase_file(name) == -1) {
                handle_error("\n#### ERROR! Cannot erase file! ####\n");
            } else {
                printf("- File '%s' erased.\n", name);
            }

        } else if (strcmp(command, "wof") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid file name.\n");
                continue;
            }
            getchar();
            if (fgets(data, sizeof(data), stdin) == NULL) {
                printf("- Invalid input.\n");
                continue;
            }
            data[strcspn(data, "\n")] = 0;
            FileHandler fh;
            fh.file_entry = locate_file(name, 0);

            if(fh.file_entry == NULL){
                printf("- File not found.\n");
                continue;
            }
            
            if (write_file(&fh, data) == -1) {
                handle_error("\n#### ERROR! Cannot write to file! ####\n");
            } else {
                printf("- Data written to '%s'.\n", name);
            }

        } else if (strcmp(command, "cat") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid input.\n");
                continue;
            }
            printf("- Reading file '%s'\n", name);
            FileHandler fh;
            fh.file_entry = locate_file(name, 0);

            if(fh.file_entry == NULL){
                printf("- File not found.\n");
                continue;
            }

            fh.pos = 0;
            int bytes_read = read_file(&fh, buff, sizeof(buff));
            buff[bytes_read] = '\0';
        
        } else if (strcmp(command, "fseek") == 0) {
            if (scanf("%s", name) != 1 || scanf("%d", &pos) != 1) {
                printf("- Invalid input.\n");
                continue;
            }
            FileHandler fh;
            fh.file_entry = locate_file(name, 0);
            if (seek_file(&fh, pos) == -1) {
                handle_error("\n#### ERROR! Cannot seek to file! ####\n");
            } else {
                printf("File '%s' seeked to position %d.\n", name, pos);
            }

        } else if (strcmp(command, "mkdir") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid directory name.\n");
                continue;
            }
            if (create_directory(name) == -1) {
                handle_error("\n#### ERROR! Cannot create directory! ####\n");
            } else {
                printf("- Directory '%s' created.\n", name);
            }

        } else if (strcmp(command, "rmdir") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid directory name.\n");
                continue;
            }
            if (erase_directory(name) == -1) {
                handle_error("\n#### ERROR! Cannot erase directory! ####\n");
            } else {
                printf("- Directory '%s' erased.\n", name);
            }

        } else if (strcmp(command, "cd") == 0) {
            if (scanf("%s", name) != 1) {
                printf("- Invalid directory name.\n");
                continue;
            }
            if (change_directory(name) == -1) {
                continue;
            } else {
                printf("- Changed to directory '%s'\n", name);
            }

        } else if (strcmp(command, "fat") == 0) {
            if (scanf("%d", &items) != 1) {
                printf("- Invalid number of items.\n");
                continue;
            }
            print_fat(items);

        } else if (strcmp(command, "ls") == 0) {
            list_directory();

        } else if (strcmp(command, "help") == 0) {
            print_help();

        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting the program...\n");
            break;

        } else {
            printf("Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    return 0;
}
