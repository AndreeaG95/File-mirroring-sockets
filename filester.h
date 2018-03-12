#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>



typedef struct file_info{
  char path[PATH_MAX];
  uint32_t size;
  int32_t timestamp;
  //uint16_t id;     // id of file to be retreived.  
}file_info;

void getFiles(file_info *files, char* path, int* length, int* max_size);

void merror(char *msg);
