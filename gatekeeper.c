#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef _LINUX
#include <sys/inotify.h>
#endif
#include <sys/types.h>
#include <arpa/inet.h>
#include <grp.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <net/if.h>
#include <ctype.h>

/* Defines */
#define FAILURE -1
#define SUCCESS 1

/* Globals */
int log_fd;
struct sockaddr_in log_addr;

/* Prototypes */
void usage(void);
void Log(char *format, ...);
int main(int argc, char * argv[]);
void sigchld();
int setup_logsocket(char * logsrvstr);
int setup_listener(char * listenstr, int * out_fd_r, int * out_fd_w);
int parse_address_string(char * instr, void * addr, size_t addr_size, unsigned short * port, int address_family);
void set_nonblock(int fd);
int is_nonblock(int fd);
int init_tcp4(unsigned short port, struct in_addr * ia4, int * server_sock;)
int init_tcp6(unsigned short port, struct in6_addr * ia6, int * server_sock);
int init_udp4(unsigned short port, struct in_addr * ia4);
int init_udp6(unsigned short port, struct in6_addr * ia6);
int accept_tcp_connection(int server_fd);
/*
 * Log functions takes in a format string and variable arguments.
 * It sends the resulting printf off to a UDP socket stored in log_buf
 * to a host/port specified in log_addr.
 *
 * We need to do this because writing to stdout/err will mess with with clients
 * that use these FD's. Basically, no printing. K?
 *
 * Will break this rule ONLY if log_fd == -1.
 */

void Log(char *format, ...)
{
    va_list ap;
    char log_buf[1024];
    va_start(ap, format);
    vsnprintf(log_buf, sizeof(log_buf)-1, format, ap);
    if (log_fd == -1)
    {
    	fprintf(stderr, log_buf);
    }
    else
    {
    	sendto(log_fd, log_buf, strlen(log_buf), 0, (struct sockaddr*)&log_addr, sizeof(log_addr));
    }
    va_end(ap);
}

/*
 * Logs usage and returns. Does not exit.
 */

void usage(void)
{
    Log("Usage:\n");
    Log("      -l <listen string> \n");
    Log("      -r <redirect string> \n");
    Log("      -o <optional log server string> \n");
    Log("      -c <optional capture host string> \n");
    Log("      -a <optional alarm in seconds> \n");
    Log("      -k <optional path to key file>\n\n");
    Log("   example usage: gatekeeper -l stdio -r stdio:/home/atmail/atmail -k /home/keys/atmail -l 10.0.0.2:2090\n");
    Log("                  gatekeeper -l tcpipv4:0.0.0.0:1234 -r stdio:/home/atmail/atmail -a 5\n");
    Log("                  gatekeeper -l tcpipv6:[::]:1234 -r stdio:/home/atmail/atmail\n\n");
}

/*
 * Entry point. Performs argument parsing and validation. Sets up log socket, listen socket, and enters
 * main client service loop.
 *
 * This program is designed to listen on one of stdio/tcp/udp and redirect to one of stdio/tcp/udp on ipv4 and ipv6.
 * Mix and match as required. These are listenstr and redirectstr arguments/variables.
 *
 * Specifying a log server is necessary for programs listening on stdio.
 * Specifying a capture host will send a copy of all traffic sent or received to the capture server.
 * Specifying an alarm value will set and alarm for that number of seconds
 * Specifying a key file will setup inotify monitoring on that key file.
 *
 */

int main(int argc, char * argv[])
{
    char * keyfile     = NULL;
    char * listenstr   = NULL;
    char * redirectstr = NULL;
    char * logsrvstr   = NULL;
    char * capsrvstr   = NULL;
    int c              = -1;
    unsigned int alarmval = 0;
    int listen_fd_r    = -1;
    int listen_fd_w    = -1;
    int remote_fd_r    = -1;
    int remote_fd_w    = -1;

    /* initialize globals */
    log_fd             = -1;
    memset(&log_addr, 0, sizeof(log_addr));

    /* at least 5 arguments to run: program, -l, listenstr, -r, redirstr */
    if (argc < 5)
    {
        usage();
        goto cleanup;
    }

    /* avoid zombie children */
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
    {
        Log("Unable to set SIGCHLD handler");
        goto cleanup;
    }

    while ((c=getopt(argc, argv, "hl:r:k:o:c:a:")) != -1)
    {
        switch (c)
        {
        case 'h':
        	/* help and exit*/
            usage();
            goto cleanup;
        case 'l':
        	/* save off listen argument string -- required*/
            listenstr = optarg;
            break;
        case 'r':
        	/* save off redirect argument string -- required*/
            redirectstr = optarg;
            break;
        case 'k':
        	/* save off keyfile argument (optional) */
            keyfile = optarg;
            break;
        case 'o':
            logsrvstr = optarg;
            break;
        case 'c':
            capsrvstr = optarg;
            break;
        case 'a':
            alarmval = atoi(optarg);
            break;
        case '?':
            usage();
            if ('l' == optopt || 'r' == optopt || 'k' == optopt || 'o' == optopt || 'c' == optopt || 'a' == optopt)
                Log("Option '-%c' requires an argument\n", optopt);
            else if(isprint(optopt))
                Log("Unknown options '-%c'\n", optopt);
            else
                Log("Unknown getopt return: 0x%x\n", c);
            goto cleanup;
        }
    }

    /* check for required arguments */
    if (listenstr == NULL)
    {
    	Log("Argument -l is required\n");
    	goto cleanup;
    }

    if (redirectstr == NULL)
    {
    	Log("Argument -r is required\n");
    	goto cleanup;
    }

    /* setup logging server globals */
    if (logsrvstr != NULL)
    {
        if (setup_logsocket(logsrvstr) == FAILURE)
        {
        	Log("Failed to setup log server socket\n");
            goto cleanup;
        }
    }

    /* set alarm if one was specified */
    if (alarmval != 0)
    {
    	alarm(alarmval);
    }

    /* setup listener fd's */
    if (setup_listener(listenstr, &listen_fd_r, &listen_fd_w) == FAILURE)
    {
        goto cleanup;
    }

    /* setup redirect fd's */

    /* start the pumps */


cleanup:
    if (listen_fd_r != -1)
        close(listen_fd_r);
    if (listen_fd_w != -1)
        close(listen_fd_w);
    if (log_fd != -1)
        close(log_fd);

    return SUCCESS;
}

/*
 * This farms exit status from forked children to avoid
 * having any zombie processes lying around
 */

void sigchld() {
    int status;
    while (wait4(-1, &status, WNOHANG, NULL) > 0) {  }
}

/*
 * Setups a socket to be used with the logging server. IPv4 only. UDP socket.
 */
int setup_logsocket(char * logsrvstr)
{
	log_addr.sin_family = AF_INET;
	if(parse_address_string(logsrvstr, &log_addr.sin_addr.s_addr, sizeof(log_addr.sin_addr.s_addr), &log_addr.sin_port, AF_INET) == SUCCESS)
	{
		log_fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (log_fd == -1)
			return FAILURE;
	}
	return SUCCESS;
}

/*
 * Takes in the listenstr and returns to fd's one for output one for input
 * in all socket types the fd's are the same value. In the case of stdin
 * these are stdin and stdout and stderr is dup2() to stdout.
 *
 * At the end of this function server sockets will have a connected socket.
 */
int setup_listener(char * listenstr, int * out_fd_r, int * out_fd_w)
{
    struct in_addr addr;
    struct in6_addr addr6;
    unsigned short port = 0;
    char str[INET6_ADDRSTRLEN];
    int fd_r = -1;
    int fd_w = -1;

    *out_fd_r = fd_r;
    *out_fd_r = fd_w;

    memset(str, 0, sizeof(str));
    memset(&addr, 0, sizeof(addr));
    memset(&addr6, 0, sizeof(addr6));

    if (strncmp("stdio", listenstr, strlen("stdio")) == 0)
    {
    	/* remove buffering on stdio */
        if (0 != setvbuf(stdin, (char*) NULL, _IONBF, 0) ||
            0 != setvbuf(stdout, (char*) NULL, _IONBF, 0) ||
            0 != setvbuf(stderr, (char*) NULL, _IONBF, 0))
        {
            return FAILURE;
        }
        fd_r = 0;
        fd_w = 1;
        /* redirect stderr to stdout so we only have 2 fd's to deal with */
        dup2(1, 2);
    }
    else if (strncmp("tcpipv4", listenstr, strlen("tcpipv4")) == 0)
    {
        if (parse_address_string(listenstr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("TCP %s:%d\n", inet_ntoa(addr), port);
        }
        else
        {
        	return FAILURE;
        }
        fd_r = fd_w = init_tcp4(port, &addr);
    }
    else if (strncmp("udpipv4", listenstr, strlen("udpipv4")) == 0)
    {
        if (parse_address_string(listenstr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("UDP %s:%d\n", inet_ntoa(addr), port);
        }
		else
		{
			return FAILURE;
		}
        fd_r = fd_w = init_udp4(port, &addr);
    }
    else if (strncmp("tcpipv6", listenstr, strlen("tcpipv6")) == 0)
    {
        if (parse_address_string(listenstr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("TCP [%s]:%d\n", str, port);
        }
		else
		{
			return FAILURE;
		}
        fd_r = fd_w = init_tcp6(port, &addr6);
    }
    else if (strncmp("udpipv6", listenstr, strlen("udpipv6")) == 0)
    {
        if (parse_address_string(listenstr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("UDP [%s]:%d\n", str, port);
        }
		else
		{
			return FAILURE;
		}
        fd_r = fd_w = init_udp6(port, &addr6);
    }
    *out_fd_r = fd_r;
    *out_fd_w = fd_w;
    return SUCCESS;

}

/*
 * Parses an "address string" that is passed in as a listen addr or remote addr. You can think of this
 * a bit like how socat handles complex strings describing a connection. Each string need to have one of
 * tcpipv4, tcpipv6, udpipv4, or udpipv6. Additional information is required though and this is specified
 * by placing a colon after the initial string. So, in the case of tcpipv4 we need to know two things --
 * an ipv4 address and a port. One example if a valid instr parameter might be "tcpipv4:127.0.0.1:8000"
 * saying that the protocol is tcpipv4 and the ip address is 127.0.0.1 and the port is 8000. This same
 * function is used for both listenstr and remotestr inputs.
 *
 * Other examples of valid input:
 *     - tcpipv4:1234                      will default addr to 0.0.0.0
 *     - tcpipv6:[::aaaa:bbbb:cccc]:80     ipv6 host with port 80 specified, ALL ipv6 hosts must be contained in []
 *     - udpipv4:192.168.0.1:1234		   ...
 *
 * instr is the full input string
 * addr is the returned in_addr4 or in_addr6  structure after a inet_pton() call
 * addr_size is the size of the addr pointer, should be sizeof(struct in_addr[4|6])
 * port is a pointer to a short value for the returned port number
 * address_family is either AF_INET4 or AF_INET6
 */
int parse_address_string(char * instr, void * addr, size_t addr_size, unsigned short * port, int address_family)
{
    char * string_start = NULL;
    char * addr_start = NULL;
    char * port_start = NULL;
    char * end = NULL;
    int tmp_port = 0;

    if (instr == NULL || port == NULL || addr == NULL || addr_size == 0 || address_family == 0)
    {
        Log("Missing parameters to parse_address_string.\n");
        return FAILURE;
    }

    if (address_family != AF_INET && address_family != AF_INET6)
    {
        Log("Invalid address family specified.\n");
        return FAILURE;
    }

    if ((addr_size != sizeof(struct in_addr) && address_family == AF_INET) ||
        (addr_size != sizeof(struct in6_addr) && address_family == AF_INET6))
    {
        Log("Invalid address size specified.\n");
        return FAILURE;
    }

    /* advance past tcpipv4 or tcpipv6 we start after the ':' */
    string_start = strchr(instr, ':');
    if (string_start == NULL)
    {
        Log("Unable to parse address string. Format as tcpipv4:1.2.3.4:1234 or tcpipv4:1234\n");
        return FAILURE;
    }
    *string_start = '\x00';
    string_start++;

    /* null terminate between arguments */
    end = strchr(string_start, ' ');
    if (end != NULL)
        *end = '\x00';

    /* parsing for IPv6 addresses */
    if (address_family == AF_INET6)
    {
        addr_start = strchr (string_start, '[');
        end = strchr (string_start, ']');
        if (addr_start == NULL && end == NULL)
        {
            /* only a port was specified */
            port_start = string_start;
            addr_start = NULL;
            memset(addr, 0, addr_size);
        }
        else if (addr_start != NULL && end != NULL)
        {
            /* remove the '[', ']', and ':' following the ']' character */
            *addr_start = '\x00';
            addr_start++;
            *end = '\x00';
            end++;
            if (*end != ':')
            {
                Log("No port specified with IPv6 address. Format as tcpipv6:[::aaaa:bbbb:cccc]:80\n");
                return FAILURE;
            }
            *end = '\x00';
            end++;
            port_start = end;
        }
        else
        {
            Log("Invalid IPv6 address specified in address string. Format as tcpipv6:[::aaaa:bbbb:cccc]:80\n");
            return FAILURE;
        }
    }
    else if (address_family == AF_INET)
    {
        port_start = strchr(string_start, ':');
        if (port_start != NULL)
        {
            addr_start = string_start;
            *port_start = '\x00';
            port_start++;
        }
        else
        {
            /* only a port was specified */
            port_start = string_start;
            addr_start = NULL;
            memset(addr, 0, addr_size);
        }
    }

    if (addr_start != NULL)
    {
        if (inet_pton(address_family, addr_start, addr) != 1)
        {
            Log("Invalid IP address specified in address string.\n");
            return FAILURE;
        }
    }

    tmp_port = atoi(port_start);
    if (tmp_port > 0 && tmp_port <= 0xffff)
    {
        *port = (unsigned short) tmp_port;
    }
    else
    {
        Log("Invalid port specification in address string.\n");
        return FAILURE;
    }
    return SUCCESS;
}

/*
 * sets the O_NONBLOCK flag on the passed in fd
 */
void set_nonblock(int fd)
{
    int fl = -1;
    int x = -1;
    fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0)
    {
        Log("fcntl F_GETFL: FD %d: %s\n", fd, strerror(errno));
        return;
    }
    x = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    if (x < 0)
    {
        Log("fcntl F_SETFL: FD %d: %s\n", fd, strerror(errno));
        return;
    }
}

/*
 * returns 1 if the fd has the O_NONBLOCK flag set
 * returns 0 if the fd does NOT have the 0_NONBLOCK flag set, or on failure
 */
int is_nonblock(int fd)
{
    int fl = -1;
    int x = -1;
    fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0)
    {
        Log("fcntl F_GETFL: FD %d: %s\n", fd, strerror(errno));
        return 0;
    }
    if (fl & O_NONBLOCK)
    {
    	return 1;
    }
    else
    {
    	return 1;
    }
}

/*
 * initializes a new tcp/ipv4 socket on the specified port, sets fd to non blocking, and accepts
 * a new connection. Sockets will be bound to the in_addr specified in ia4. server socket is returned
 * in the int * for server sock and the client socket is returned as the function return.
 */
int init_tcp4(unsigned short port, struct in_addr * ia4, int * server_sock)
{
    int fd = -1;
    int client_sock;
    struct sockaddr_in my_addr;
    struct sockaddr client_addr;
    socklen_t client_len = sizof(client_addr);
    int one = 1;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    memcpy(&my_addr.sin_addr, (void *) ia4, sizeof(my_addr.sin_addr));

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    set_nonblock(fd);
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    if (listen(fd, 40) == -1)
    {
        Log("Unable to listen on socket\n");
        return FAILURE;
    }
    client_sock = accept_tcp_connection(fd);
    if (server_sock != NULL)
    {
    	*server_sock = fd;
    }
    else
    {
    	close(fd);
    }
    return client_sock;
}

/* takes in port to lisent on as an IPv6 socket, will bind to address in in6_addr ia6 and
 * returns connected client as function return. server sock is returned as the int *
 */

int init_tcp6(unsigned short port, struct in6_addr * ia6, int * server_sock)
{
    int fd = -1;
    int client_sock = -1;
    struct sockaddr_in6 my_addr;
    int one = 1;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(port);
    memcpy(&my_addr.sin6_addr, (void *) ia6, sizeof(my_addr.sin6_addr));

    fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd == -1)
    {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    set_nonblock(fd);
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    if (listen(fd, 40) == -1)
    {
        Log("Unable to listen on socket\n");
        return FAILURE;
    }
    client_sock = accept_tcp_connection(fd);
	if (server_sock != NULL)
	{
		*server_sock = fd;
	}
	else
	{
		close(fd);
	}
	return client_sock;
}

/*
 * sets up a udp / ipv4 socket for listening on specified port on address specified in in_addr ia4
 */
int init_udp4(unsigned short port, struct in_addr * ia4)
{
    int fd = -1;
    struct sockaddr_in my_addr;
    int one = 1;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    memcpy(&my_addr.sin_addr, (void *) ia4, sizeof(my_addr.sin_addr));

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    set_nonblock(fd);
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    return fd;
}
/*
 * sets up a udp / ipv6 socket for listening on specified port on address specified in in_addr ia6
 */

int init_udp6(unsigned short port, struct in6_addr * ia6)
{
    int fd = -1;
    struct sockaddr_in6 my_addr;
    int one = 1;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(port);
    memcpy(&my_addr.sin6_addr, (void *) ia6, sizeof(my_addr.sin6_addr));

    fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    set_nonblock(fd);
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    return fd;
}

/*
 * accepts new TCP connection from the fd specified, socked should be bound and listening
 * returns new client fd. discards connected client in_addr after logging.
 */

int accept_tcp_connection(int server_fd)
{
	int client_fd = -1;
	return client_fd;
}
