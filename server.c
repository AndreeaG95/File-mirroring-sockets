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
#include "netio.h"
#include <string.h>


#define SERVER_PORT 5678
int length;

void merror(char *msg)
{
  fputs(msg, stderr);
  exit(1);
}

void getFiles(file_info *files, char* path){
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

    if(S_ISDIR(mstat.st_mode)){
      strncpy(files[length].path, newpath, sizeof(files[length].path)); 
      
      files[length].size = (uint32_t)mstat.st_size;
      files[length].timestamp = (int32_t)mstat.st_mtime;
      files[length].permissions = (int)mstat.st_mode; 
      
      //files++;
      length++;
      getFiles(files, newpath);
    }else if(S_ISREG(mstat.st_mode) || S_ISLNK(mstat.st_mode)){
      strncpy(files[length].path, newpath, sizeof(files[length].path)); 
    
      //puts(files->path);
      files[length].size = (uint32_t)mstat.st_size;
      files[length].timestamp = (int32_t)mstat.st_mtime;
      files[length].permissions = (int)mstat.st_mode; 
      
      //files++;
      length++;
    }
  }

  free(newpath);
}

int main(void){

  int sockfd, connfd;
  struct sockaddr_in local_addr, rmt_addr;
  socklen_t rlen = sizeof(rmt_addr);
  int clientSize = 10;
  struct sockaddr_in *client_list = malloc(clientSize*sizeof(struct sockaddr_in));
  file_info *files = malloc(100*sizeof(file_info));
  char* root = "root";
  
  //chdir() mabey.
  getFiles(files, root);
 
  if(client_list == NULL)
    merror("Could not allocate memory for server clients."); 
  
  // PF_INET=TCP/UP, 0=implicit.
  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if(sockfd == -1){
    merror("Unable to create server socket");
  }

  // Set the server to listen to all interfaces on the selected port.
  if (set_addr(&local_addr, NULL, INADDR_ANY, SERVER_PORT) == -1)
    merror("Unable to get server address");
  
  if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
      merror("Unable to binf to socket");
  
  if (listen(sockfd, 5) == -1)
    merror("Unable to listen on socket");

  int clients=0;
  while(clients < 100){
    
    // Create a new socket for each connection.
    connfd = accept(sockfd, (struct sockaddr *)&rmt_addr, &rlen);
    
    if(connfd == -1)
      merror("Could not accept connection");

    client_list[clients] = rmt_addr;
    clients++;

    if(clients >= clientSize){
      clientSize = clientSize + clientSize/2;
      client_list = realloc(client_list, clientSize);

      if (client_list == NULL)
	merror("Unable to add memory for client");
    }    

    for(int i=0; i<10; i++){
      puts(files[i].path);
    }

    // Send tree status.
    printf("SIZE: %d \n",length);
    stream_write(connfd, (void *)files, sizeof(file_info) * length);

    close(connfd);
  }

  free(files);
  free(client_list);
  close(sockfd);
  exit(0);

  return 0;
}
