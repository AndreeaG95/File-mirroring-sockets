#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

// set_addr creates a structure of type sockaddr_in with 
// the desired address and port
int set_addr(struct sockaddr_in *addr, char *name, u_int32_t inaddr, short port);

int stream_read(int sockfd, void *buff, int len);

int stream_write(int sockfd, const void *buff, int len);

int send_file(int sockfd, const char *file);

int get_file(int sockfd, const char *file);
