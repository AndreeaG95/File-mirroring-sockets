#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include "netio.h"
#include "filester.h"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     5678

int main(int argc, char** argv){
  int sockfd;
  char root[30];
  struct sockaddr_in local_addr;

  if(argc != 2)
    merror("Please add root name as first argument");

  strncpy(root, argv[1], sizeof(root));

  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if(sockfd == -1){
    merror("Unable to create client socket");
  }

  set_addr(&local_addr, NULL, INADDR_ANY, 0);

  // We also use bind on client because we want to connect on a specific port.
  bind(sockfd, (struct sockaddr *)& local_addr , sizeof(local_addr) );

  if (set_addr(&local_addr, SERVER_ADDRESS, 0, SERVER_PORT) == -1)
    merror("Unable to get client address");

  if(connect(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
    merror("Unable to connect to socket");
  
  file_info info[100];
  stream_read(sockfd, (void*) info, sizeof(file_info)*10);

  for(int i=0; i<10; i++){
    puts(info[i].path);
  }

  close(sockfd);  
}
