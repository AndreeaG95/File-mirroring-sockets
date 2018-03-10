#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct file_info{
  char path[256];
  uint32_t size;
  int32_t timestamp;
  int permissions;
}file_info;

// set_addr creates a structure of type sockaddr_in with 
// the desired address and port
int set_addr(struct sockaddr_in *addr, char *name, u_int32_t inaddr, short port);

int stream_read(int sockfd, char *buff, int len);

int stream_write(int sockfd, char *buff, int len);
