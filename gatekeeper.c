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

#define FAILURE -1
#define SUCCESS 1

int log_fd;

void usage(void);
void Log(char *format, ...);
int main(int argc, char * argv[]);
int setup_logsocket(char * logsrvstr);
int setup_listener(char * listenstr, int * out_fd_r, int * out_fd_w);
int parse_address_string(char * instr, void * addr, size_t addr_size, unsigned short * port, int address_family);
void set_nonblock(int fd);
int init_tcp4(unsigned short port, struct in_addr * ia4);
int init_tcp6(unsigned short port, struct in6_addr * ia6);
int init_udp4(unsigned short port, struct in_addr * ia4);
int init_udp6(unsigned short port, struct in6_addr * ia6);

void Log(char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    (void) vfprintf(stderr, format, ap);
    va_end(ap);
}

void usage(void)
{
    Log("Usage:\n");
    Log("      -l <listen string> \n");
    Log("      -r <redirect string> \n");
    Log("      -o <optional log server string> \n");
    Log("      -k <optional path to key file>\n\n");
    Log("   example usage: gatekeeper -l stdio -r stdio:/home/atmail/atmail -k /home/keys/atmail -l 10.0.0.2:2090\n");
    Log("                  gatekeeper -l tcpipv4:0.0.0.0:1234 -r stdio:/home/atmail/atmail\n\n");
    Log("                  gatekeeper -l tcpipv6:[::]:1234 -r stdio:/home/atmail/atmail\n\n");
}

int main(int argc, char * argv[])
{
    char * keyfile     = NULL;
    char * listenstr   = NULL;
    char * redirectstr = NULL;
    char * logsrvstr   = NULL;
    int32_t c          = -1;
    int listen_fd_r    = -1;
    int listen_fd_w    = -1;
    int remote_fd_r    = -1;
    int remote_fd_w    = -1;
    log_fd             = -1;

    if (argc < 5)
    {
        usage();
        exit(1);
    }
    while ((c=getopt(argc, argv, "hl:r:k:o:")) != -1)
    {
        switch (c)
        {
        case 'h':
            usage();
            return SUCCESS;
        case 'l':
            listenstr = optarg;
            break;
        case 'r':
            redirectstr = optarg;
            break;
        case 'k':
            keyfile = optarg;
            break;
        case 'o':
            logsrvstr = optarg;
            break;
        case '?':
            usage();
            if ('l' == optopt || 'r' == optopt || 'k' == optopt || 'o' == optopt)
                Log("Option '-%c' requires an argument\n", optopt);
            else if(isprint(optopt))
                Log("Unknown options '-%c'\n", optopt);
            else
                Log("Unknown getopt return: 0x%x\n", c);
            return SUCCESS;
        }
    }

    if (logsrvstr != NULL)
    {
        if (setup_logsocket(logsrvstr) == FAILURE)
        {
            goto failure;
        }
    }

    if (setup_listener(listenstr, &listen_fd_r, &listen_fd_w) == FAILURE)
    {
        goto failure;
    }

    return SUCCESS;
failure:
    if (listen_fd_r != -1)
        close(listen_fd_r);
    if (listen_fd_w != -1)
        close(listen_fd_w);
    if (log_fd != -1)
        close(log_fd);
    return FAILURE; 
}

int setup_logsocket(char * logsrvstr)
{

}
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
        if (0 != setvbuf(stdin, (char*) NULL, _IONBF, 0) ||
            0 != setvbuf(stdout, (char*) NULL, _IONBF, 0) ||
            0 != setvbuf(stderr, (char*) NULL, _IONBF, 0))
        {
            return FAILURE;
        }
        fd_r = 0;
        fd_w = 1;
    }
    else if (strncmp("tcpipv4", listenstr, strlen("tcpipv4")) == 0)
    {
        if (parse_address_string(listenstr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("TCP %s:%d\n", inet_ntoa(addr), port);
        } else { return FAILURE; }
        fd_r = fd_w = init_tcp4(port, &addr);
    }
    else if (strncmp("udpipv4", listenstr, strlen("udpipv4")) == 0)
    {
        if (parse_address_string(listenstr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("UDP %s:%d\n", inet_ntoa(addr), port);
        } else { return FAILURE; }
        fd_r = fd_w = init_udp4(port, &addr);
    }
    else if (strncmp("tcpipv6", listenstr, strlen("tcpipv6")) == 0)
    {
        if (parse_address_string(listenstr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("TCP [%s]:%d\n", str, port);
        } else { return FAILURE; }
        fd_r = fd_w = init_tcp6(port, &addr6);
    }
    else if (strncmp("udpipv6", listenstr, strlen("udpipv6")) == 0)
    {
        if (parse_address_string(listenstr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("UDP [%s]:%d\n", str, port);
        } else { return FAILURE; }
        fd_r = fd_w = init_udp6(port, &addr6);
    }
    *out_fd_r = fd_r;
    *out_fd_w = fd_w;
    return SUCCESS;

}


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

int init_tcp4(unsigned short port, struct in_addr * ia4)
{
    int fd = -1;
    struct sockaddr_in my_addr;
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
    return fd;
}

int init_tcp6(unsigned short port, struct in6_addr * ia6)
{
    int fd = -1;
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
    return fd;
}

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
