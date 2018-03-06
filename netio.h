#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

// set_addr creates a structure of type sockaddr_in with 
// the desired address and port
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

int stream_read(int sockfd, char *buff, int len){
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

int stream_write(int sockfd, char *buff, int len){
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
