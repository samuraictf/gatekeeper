#ifndef _GATEKEEPER_H
#define _GATEKEEPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <ctype.h>
#include <grp.h>
#include <ifaddrs.h>

#ifdef _LINUX
#include <sys/inotify.h>
#endif
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

/* Defines */
#define FAILURE -1
#define SUCCESS 1
#define LOCAL 1
#define REMOTE 0
#define REMOTE_RAND_ENV 2

/* Globals */
int log_fd;
struct sockaddr_in log_addr;

/* Prototypes */
void usage(void);
void Log(char *format, ...);
int main(int argc, char * argv[]);
void sigchld();
int setup_logsocket(char * logsrvstr);
int setup_connection(char * listenstr, int * out_fd_r, int * out_fd_w, int local);
int parse_address_string(char * instr, void * addr, size_t addr_size, unsigned short * port, int address_family);
char ** build_rand_envp();

void set_nonblock(int fd);
int is_nonblock(int fd);
int init_tcp4(unsigned short port, struct in_addr * ia4, int * server_sock);
int init_tcp6(unsigned short port, struct in6_addr * ia6, int * server_sock);
int init_udp4(unsigned short port, struct in_addr * ia4);
int init_udp6(unsigned short port, struct in6_addr * ia6);
int accept_tcp_connection(int server_fd, int address_family);
#endif
