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

#include "netio.h"
#include "filester.h"

#define SERVER_PORT 5678


int cmp(const void* a, const void* b)
{
  return strcmp( ((file_info *)a)->path, ((file_info *)b)->path );
}

int main(int argc, char* argv[]){
  int sockfd, connfd;
  struct sockaddr_in local_addr, rmt_addr;
  int max_length = 100; 
  socklen_t rlen = sizeof(rmt_addr);

  file_info *files = malloc(max_length*sizeof(file_info));
  
  if (argc != 2){
	merror("Please call: exename softwareFolder");
  }

  char* root = argv[1];
  
  chdir(root);
  
   
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

  while(1){
    
    // Create a new socket for each connection.
    connfd = accept(sockfd, (struct sockaddr *)&rmt_addr, &rlen);
    
    if(connfd == -1)
      merror("Could not accept connection");
    
    pid_t pid = fork();

    if(pid == 0) // new process
      {
	int length = 0;
	max_length = 100;
	getFiles(files, ".", &length, &max_length);
	qsort(files, length, sizeof(file_info), cmp);
  
	// talk with client
        stream_write(connfd, (void *)&length, sizeof(int));
	stream_write(connfd, (void *)files, sizeof(file_info) * length);
	
	//send_file(sockfd, "./test.txt");
	uint32_t command=0;
	uint16_t index;
	int nread;
	while(0 < (nread = stream_read(connfd, &command, sizeof command)))
	  {
	    if(command == 0xF00D)
	      {
		printf("got food\n");
		
		stream_read(connfd, &index, sizeof index); 

		printf("Sending: %s\n", files[index].path);
		send_file(connfd, files[index].path);
	      }    
	  }
	printf("Connection ending\n");
	
	// Send tree status.
	printf("SIZE: %d \n",length);
	
	free(files);
   

	exit(0);
      }
    else if(pid == -1) // fork error
      {
	merror("Fork error");
      }

    else  // parent process
      {
	close(connfd);
      }

  }

 
  close(sockfd);
  exit(0);

  return 0;
}
