#include "gatekeeper.h"

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
        return 0;
    }
}

/*
 * initializes a new tcp/ipv4 socket on the specified port, sets fd to non blocking, and accepts
 * a new connection. Sockets will be bound to the in_addr specified in ia4. server socket is returned
 * in the int * for server sock and the client socket is returned as the function return. client
 * socket is in nonblocking mode.
 */
int init_tcp4(unsigned short port, struct in_addr * ia4, int * server_sock)
{
    int fd = -1;
    int client_sock;
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
    client_sock = accept_tcp_connection(fd, AF_INET);
    set_nonblock(client_sock);
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

/* takes in port to listen on as an IPv6 socket, will bind to address in in6_addr ia6 and
 * returns connected client as function return. server sock is returned as the int *
 * if server_sock is not null it will be returned in the pointer otherwise the server
 * socket will be closed in this function. client socket is in nonblocking mode.
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
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    client_sock = accept_tcp_connection(fd, AF_INET6);
    set_nonblock(client_sock);
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

int accept_tcp_connection(int server_fd, int address_family)
{
	struct sockaddr_in client_addr4;
	struct sockaddr_in6 client_addr6;
	socklen_t len = 0;
    int client_fd = -1;
    memset(&client_addr4, 0, sizeof(client_addr4));
    memset(&client_addr6, 0, sizeof(client_addr6));
    if (0 != listen(server_fd, 20))
    {
    	return FAILURE;
    }
    if (address_family == AF_INET)
    {
    	len = sizeof(client_addr4);
    	client_fd = accept(server_fd, (struct sockaddr *) &client_addr4, &len);
    }
    else if (address_family == AF_INET6)
    {
    	len = sizeof(client_addr6);
    	client_fd = accept(server_fd, (struct sockaddr *) &client_addr6, &len);
    }
    else
    {
    	return FAILURE;
    }
    set_nonblock(client_fd);
    return client_fd;
}
