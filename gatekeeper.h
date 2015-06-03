#ifndef _GATEKEEPER_H
#define _GATEKEEPER_H
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pcre.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ringbuffer.h"
#include "ctf.h"

/* Defines */
#define FAILURE EXIT_FAILURE
#define SUCCESS EXIT_SUCCESS
#define LOCAL 1
#define REMOTE 0
#define REMOTE_RAND_ENV 2
#define RECVBUF_SIZE 4096


/* typedefs / structs */
typedef struct pcre_list pcre_list_t;
struct pcre_list {
	pcre_list_t *next;
	pcre *re;
};

typedef struct _interface_ip_list {
	void * next;
	char type;
	char addr[64];
	char name[24];
	struct in_addr ia4;
	struct in6_addr ia6;
} interface_ip_list, *pinterface_ip_list;

/* Globals */
extern int log_fd;
extern struct in_addr log_addr;
extern pcre_list_t *pcre_inputs;
extern int num_pcre_inputs;
extern int debugging;
extern int verbose;
extern pinterface_ip_list if_list;

/* Prototypes */
void usage(void);
#define Log(...) ctf_writef(log_fd, __VA_ARGS__)

int main(int argc, char * argv[]);
void sigchld();
int setup_logsocket(char * logsrvstr);
int setup_connection(char * listenstr, int * out_fd_r, int * out_fd_w, int type);
int parse_address_string(char * instr, void * addr, size_t addr_size, unsigned short * port, int address_family);
char ** build_rand_envp();

void set_nonblock(int fd);
int is_nonblock(int fd);
int init_tcp4(unsigned short port, struct in_addr * ia4, int * server_sock);
int init_tcp6(unsigned short port, struct in6_addr * ia6, int * server_sock);
int init_udp4(unsigned short port, struct in_addr * ia4);
int init_udp6(unsigned short port, struct in6_addr * ia6);
int connect_ipv4(int type, unsigned short port, struct in_addr * ia4);
int connect_ipv6(int type, unsigned short port, struct in6_addr * ia6);
int accept_tcp_connection(int server_fd, int address_family);
int accept_udp_connection(int client_fd, int address_family);

int list_add(pcre *re, pcre_list_t **head);
void free_list(pcre_list_t *list);
int parse_pcre_inputs(const char *fname);
int check_for_match(char *buf, int num_bytes);
#endif
