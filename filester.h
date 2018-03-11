#include <stdio.h>
#include <sys/types.h>

typedef struct file_info{
  char path[256];
  uint32_t size;
  int32_t timestamp;
}file_info;

void getFiles(file_info *files, char* path, int* length);

void merror(char *msg);
