#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "netio.h"

#define SERVER_PORT 5678

void merror(char *msg)
{
  fputs(msg, stderr);
  exit(1);
}

int main(void){

  int fd, sockfd, connfd;
  struct sockaddr_in local_addr, rmt_addr;
  int nread, nfis; 
  socklen_t rlen = sizeof(rmt_addr);
  char buff[30];
  
  // PF_INET=TCP/UP, 0=implicit.
  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if(sockfd == -1){
    merror("Unable to create server socket");
  }

  // set the server to listen to all interfaces on the selected port.
  if (set_addr(&local_addr, NULL, INADDR_ANY, SERVER_PORT) == -1)
    merror("Unable to get server address");
  
  if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1)
      merror("Unable to binf to socket");
  
  if (listen(sockfd, 5) == -1)
    merror("Unable to listen on socket");

  int clients=1;
  while(clients < 10){
    // create a new socket for each connection.
    connfd = accept(sockfd, (struct sockaddr *)&rmt_addr, &rlen);

    if(connfd == -1)
      merror("Could not accept connection");

    clients++;
    nread = stream_read(connfd, (void*) buff, 10);
    puts(buff);

    close(connfd);
    close(fd);
  }

  close(sockfd);
  exit(0);

  return 0;
}
