#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "netio.h"
#include<stdio.h>
#include <fcntl.h>
#include <time.h>

#define MAXBUF 1024

int set_addr(struct sockaddr_in *addr, char *name, u_int32_t inaddr, short port){

  // Represents an entry in the host db.
  struct hostent *h;

  memset((void *)addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  if(name != NULL){
    h = gethostbyname(name);
    if (h == NULL){
      return -1;
    }

    // set the first address from the list.
    addr->sin_addr.s_addr = *(u_int32_t *) h->h_addr_list[0];
    
  }else{
    addr->sin_addr.s_addr = htonl(inaddr);
  }

  // htons converts format host to short.
  addr->sin_port = htons(port);
  return 0;
}

int stream_read(int sockfd, void *buff, int len){
  int nread;
  int remain = len;
  
  while(remain > 0){
    if( -1 == (nread = read(sockfd, buff, remain)) )
      return -1;
    if(nread == 0)
      break;
    remain -= nread;
    buff += nread;
  }
  return len - remain;
}

int stream_write(int sockfd, const void *buff, int len){
  int nrw;
  int rem = len;

  while(rem > 0){
    if ( -1 == (nrw = write(sockfd, buff, rem)))
      return -1;
    rem -= nrw;
    buff += nrw;
  }
  return len-rem;
}


int send_file(int sockfd, const char *file)
{
  int fd = open(file, O_RDONLY);
  
  if(fd == -1) {
    printf("Could not open %s\n", file);
    //send error code
    return -1;
  }
  int nread;
  char buf[MAXBUF];

  struct stat bstat;
  fstat(fd, &bstat);
  //  uint32_t mode = bstat.st_mode;

  stream_write(sockfd, &bstat.st_size, sizeof bstat.st_size);
  stream_write(sockfd, &bstat.st_mode, sizeof bstat.st_mode);
  stream_write(sockfd, &bstat.st_mtime, sizeof bstat.st_mtime);


  while(0 < (nread = read(fd, buf, MAXBUF)))
    {
      stream_write(sockfd, buf, nread);
    }
  if (nread < 0)
    {
      printf("Error reading from file.\n");
      //send error code
      //      return -1;
    }
  
  close(fd);
  
  return 0;
}

int request_file(int sockfd, const char* file)
{
  uint32_t rq = 0xF00D;
  uint32_t size = strlen(file);


  printf("Requesting %s\n", file);
  stream_write(sockfd, &rq, sizeof rq);
  stream_write(sockfd, &size, sizeof size);
  stream_write(sockfd, file, size);

  //  stream_read(sockfd, filesize, sizeof filesize);

  return 0;
}


int get_file(int sockfd, const char* file)
{

  if (request_file(sockfd, file))
    return -1;

  off_t st_size;
  time_t mtime;
  mode_t st_mode;

  stream_read(sockfd, &st_size, sizeof st_size);
  stream_read(sockfd, &st_mode, sizeof st_mode);
  stream_read(sockfd, &mtime, sizeof mtime);
  printf("%d %X %X\n", st_size, st_mode, mtime);



  int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 00644);

  if (fd == -1)
    {
      printf("Error creating %s\n", file);
      return -1;
    }

  
  int nread;
  char buf[MAXBUF];
  
  
  while(st_size > 0)
    { 
      nread = stream_read(sockfd, buf, st_size < MAXBUF ? st_size : MAXBUF);
      st_size -= MAXBUF;
      if (nread  < 1) {
	puts("Error reading\n");
	break;
      }
      if (-1 == write(fd, buf, nread)) 
	{
	  printf("Error writing to %s\n", file);
	  close(fd);
	  return -1;
	}
    }
  printf("Got file %s\n", file);
  close(fd);
  if (nread < 0)
    return -1;
  else
    return 0;
}

