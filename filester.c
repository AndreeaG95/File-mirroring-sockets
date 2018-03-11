#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include "filester.h"
#include "netio.h"

void merror(char *msg)
{
  fputs(msg, stderr);
  exit(1);
}

void getFiles(file_info *files, char* path, int* length, int* max_size ){
  DIR *d;
  struct dirent *sdir;
  struct stat mstat;
  char* newpath = malloc(strlen(path) + 30);

  if(( d = opendir(path)) == NULL ){
    merror("Could not open directory\n");
    closedir(d);
  }

  while((sdir = readdir(d)) != NULL){
    sprintf(newpath, "%s/%s", path, sdir->d_name);
   
    lstat(newpath, &mstat);
    if((strcmp(sdir->d_name, "..") == 0) || (strcmp(sdir->d_name, ".") == 0 ))
       continue;

    // Expand memory for more files.    
    if(*length >= *max_size){
      *max_size = (*max_size) + (*max_size)/2;
      if ( (files = realloc(files, (*max_size)*sizeof(file_info))) == NULL)
	   merror("Could not allocate memory for files");
    }

    if(S_ISDIR(mstat.st_mode)){
      strncpy(files[*length].path, newpath, sizeof(files[*length].path)); 
      
      files[*length].size = (uint32_t)mstat.st_size;
      files[*length].timestamp = (int32_t)mstat.st_mtime;
      
      (*length)++;
      getFiles(files, newpath, length, max_size);
    }else if(S_ISREG(mstat.st_mode) || S_ISLNK(mstat.st_mode)){
      strncpy(files[*length].path, newpath, sizeof(files[*length].path)); 
    
      files[*length].size = (uint32_t)mstat.st_size;
      files[*length].timestamp = (int32_t)mstat.st_mtime;
      
      (*length)++;
    }
  }

  free(newpath);
}
