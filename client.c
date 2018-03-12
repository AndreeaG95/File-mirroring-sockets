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

//#define SERVER_ADDRESS "192.168.0.117"
#define SERVER_PORT     5678

/* 
 *isPresentOnServer: checks if a file is present on the server version and returns the index of the file. 
 * Otherwise returns -1.
 */
int isPresentOnServer(file_info local_file, file_info* server_files, int length)
{
  for(int i=0; i<length; i++)
    if(strcmp(local_file.path, server_files[i].path) == 0)
      return i;
  
  return -1;
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
  for(int i=0; i<length; i++){
    if(strcmp(server_file.path, local_files[i].path) == 0)
      return &local_files[i];
  }
  
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
  char *SERVER_ADDRESS;

  if(argc < 3)
    {
      printf("Please call: %s localFolder serverIp\n", argv[0]);
      exit(0);
    }

  int delete = 0;
  if (argc > 3 && !strcmp(argv[3], "-d"))
    delete = 1;
  else {
    puts("Delete flag not specified.");
  }
  
  
  SERVER_ADDRESS = argv[2];

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
  
  uint32_t length = 0;
  uint32_t max_length = 100;

  file_info *local_version = malloc(max_length * sizeof(file_info));
  
  chdir(root);

  getFiles(local_version, ".", &length, &max_length);
  qsort(local_version, length, sizeof(file_info), cmp);   

  uint32_t server_files_length;
  file_info* server_files;
  stream_read(sockfd, (void*)&server_files_length, sizeof(server_files_length));

  //  printf("size= %d\n ", server_files_length);
  
  server_files = malloc(server_files_length * sizeof(file_info));
  if (!server_files)
    merror("Not enough memory.");

  for(int i = 0; i < server_files_length; i++)
    get_fileinfo(sockfd, server_files + i);

  qsort(server_files, server_files_length, sizeof(file_info), cmp);
  for(int i=0; i<server_files_length; i++){
    puts(server_files[i].path);
    //printf("%X %X\n", server_files[i].size, server_files[i].timestamp);
  }
  puts("------------");
  for(int i=0; i<length; i++){
    puts(local_version[i].path);
    //printf("%X %X\n", local_version[i].size, local_version[i].timestamp);
  }

  // Delete local files that are not on server or get them if they are modified.
  int fileOnServer;
  for(int i=0; i<length; i++)
    {

      if( (fileOnServer = isPresentOnServer(local_version[i], server_files, server_files_length)) == -1)
	{
	  if (delete)
	    {
	      printf("Deleting %s\n", local_version[i].path);
	      if(remove_file(local_version[i].path) != 0)
		printf("Could not delete file: %s\n", local_version[i].path);
	    }
	  else
	    {
	      printf("Not on server: %s\n", local_version[i].path);
	    }
	}
      else
	{
	  if(dateModified(local_version[i], server_files[fileOnServer]) || sizeDifferent(local_version[i], server_files[fileOnServer])){
	    puts("modi");
	    get_file(sockfd, server_files[fileOnServer].path, fileOnServer);
	  }
      
	}
      
    }

  // Get the files from the server that are not on client.
  for(int i=0; i<server_files_length; i++){

    if( isOnClient(server_files[i], local_version, length)  == NULL)
      {
	puts("missing");
	get_file(sockfd, server_files[i].path, i);
      }
  }

  close(sockfd);  
}
