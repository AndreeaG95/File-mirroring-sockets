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

/* 
 *isPresentOnServer: checks if a file is present on the server version and returns it. 
 * Otherwise returns null.
 */
file_info* isPresentOnServer(file_info local_file, file_info* server_files, int length)
{
  for(int i=0; i<length; i++)
    if(strcmp(local_file.path, server_files[i].path) == 0)
      return &server_files[i];
  
  return NULL;
}

/*
 * dateModified: returns 0 if the file on the server has the same timestamp as the local one
 * and the difference otherwise.
 */
int dateModified(file_info local, file_info server)
{
  return local.timestamp - server.timestamp;
}

/*
 * sizeDifferent: returns 0 if the file on the server has the same size ad the local one
 * and an the difference otherwise
 */
int sizeDifferent(file_info local, file_info server)
{
  return local.size - server.size;
}

/* 
 *isOnClient: checks if a file from the server is present on the client version and returns it. 
 * Otherwise returns null.
 */
file_info* isOnClient(file_info server_file, file_info* local_files, int length)
{
  for(int i=0; i<length; i++)
    if(strcmp(server_file.path, local_files[i].path) == 0)
      return &local_files[i];
  
  return NULL;
}


int cmp(const void* a, const void* b)
{
  return strcmp( ((file_info *)a)->path, ((file_info *)b)->path );
}

int main(int argc, char** argv)
{
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
  
  file_info *local_version = malloc(100*sizeof(file_info));
  int length = 0;
  chdir(root);

  getFiles(local_version, ".", &length);
  qsort(local_version, length, sizeof(file_info), cmp);   

  file_info server_files[100];
  int server_files_length;
  stream_read(sockfd, (void*)&server_files_length, sizeof(int));
  printf("size= %d ", server_files_length);

  stream_read(sockfd, (void*)server_files, sizeof(file_info)*server_files_length);

  qsort(server_files, server_files_length, sizeof(file_info), cmp);
  for(int i=0; i<server_files_length; i++){
    puts(server_files[i].path);
  }
  
  file_info *sfile;
  // Delete local files that are not on server or get them if they are modified.
  for(int i=0; i<length; i++)
    {

      if( (sfile = isPresentOnServer(local_version[i], server_files, 10)) == NULL)
	{

	  if(remove(local_version[i].path) != 0)
	    if(rmdir(local_version[i].path) != 0)
	      printf("Could not delete file: %s", local_version[i].path);
	}
      else
	{
	  file_info server_file = *sfile;
	  if(dateModified(local_version[i], server_file) || sizeDifferent(local_version[i], server_file)){
	    ;
	    //TODO(): get desired file from the server and replace it.
	  }
      
	}
      
    }

  // Get the files from the server that are not on client.
  for(int i=0; i<10; i++){

    if( (sfile = isOnClient(server_files[i], local_version, length) ) == NULL)
      {
	;
      }
      // TODO(): get the file from the server.
  }
    
  close(sockfd);  
}
