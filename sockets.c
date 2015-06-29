#include "gatekeeper.h"

/*
 * sets the O_NONBLOCK flag on the passed in fd
 */
void set_nonblock(int fd)
{
    int fl = -1;
    int x = -1;
    fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) {
        Log("fcntl F_GETFL: FD %d: %s\n", fd, strerror(errno));
        return;
    }
    x = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    if (x < 0) {
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
    if (fl < 0) {
        Log("fcntl F_GETFL: FD %d: %s\n", fd, strerror(errno));
        return 0;
    }
    if (fl & O_NONBLOCK) {
        return 1;
    } else {
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
    int pid;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    memcpy(&my_addr.sin_addr, (void *) ia4, sizeof(my_addr.sin_addr));

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    if (listen(fd, 40) == -1) {
        Log("Unable to listen on socket\n");
        return FAILURE;
    }
    while (1) {
        client_sock = accept_tcp_connection(fd, AF_INET);
        pid = fork();
        if (pid == -1) {
            Log("Error on fork() in init_tcp4()\n");
            return FAILURE;
        } else if (pid == 0) {
            /* child */
            set_nonblock(client_sock);
            /* not sure of the point of returning server_sock */
            if (server_sock != NULL) {
                *server_sock = fd;
            } else {
                close(fd);
            }
            return client_sock;
        } else {
            /* parent */
            close(client_sock);
        }
    }
    /* should never reach this */
    return FAILURE;
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
    int pid;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(port);
    memcpy(&my_addr.sin6_addr, (void *) ia6, sizeof(my_addr.sin6_addr));

    fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd == -1) {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    while (1) {
        client_sock = accept_tcp_connection(fd, AF_INET6);
        pid = fork();
        if (pid == -1) {
            Log("Error on fork() in init_tcp6()\n");
            return FAILURE;
        } else if (pid == 0) {
            /* child */
            set_nonblock(client_sock);
            /* not sure of the point of returning server_sock */
            if (server_sock != NULL) {
                *server_sock = fd;
            } else {
                close(fd);
            }
            return client_sock;
        } else {
            /* parent */
            close(client_sock);
        }
    }
    /* should never reach this */
    return FAILURE;
}

/*
 * sets up a udp / ipv4 socket for listening on specified port on address specified in in_addr ia4
 */
int init_udp4(unsigned short port, struct in_addr * ia4)
{
    int fd = -1;
    struct sockaddr_in my_addr;
    int one = 1;
    int pid;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    memcpy(&my_addr.sin_addr, (void *) ia4, sizeof(my_addr.sin_addr));

    while (1) {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1) {
            Log("Unable to create socket\n");
            return FAILURE;
        }
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
            Log("Unable to set SO_REUSEADDR\n");
            return FAILURE;
        }
        if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
            Log("Unable to bind socket\n");
            return FAILURE;
        }
        /* block until we've "accepted" a connection
         * (see accept_udp_connection() for details)
         */
        if (accept_udp_connection(fd, AF_INET) == FAILURE) {
            Log("Unable to accept UDP connection in init_udp4()\n");
            return FAILURE;
        }
        pid = fork();
        if (pid == -1) {
            Log("Error on fork() in init_udp4()\n");
            return FAILURE;
        } else if (pid == 0) {
            /* child */
            return fd;
        } else {
            /* parent */
            close(fd);
        }
    }
    /* should never reach this */
    return FAILURE;
}

/*
 * sets up a udp / ipv6 socket for listening on specified port on address specified in in_addr ia6
 */
int init_udp6(unsigned short port, struct in6_addr * ia6)
{
    int fd = -1;
    struct sockaddr_in6 my_addr;
    int one = 1;
    int pid;

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(port);
    memcpy(&my_addr.sin6_addr, (void *) ia6, sizeof(my_addr.sin6_addr));

    fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (fd == -1) {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1) {
        Log("Unable to set SO_REUSEADDR\n");
        return FAILURE;
    }
    if (bind(fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1) {
        Log("Unable to bind socket\n");
        return FAILURE;
    }
    /* block until we've "accepted" a connection
     * (see accept_udp_connection() for details)
     */
    if (accept_udp_connection(fd, AF_INET6) == FAILURE) {
        Log("Unable to accept UDP connection in init_udp6()\n");
        return FAILURE;
    }
    pid = fork();
    if (pid == -1) {
        Log("Error on fork() in init_udp6()\n");
        return FAILURE;
    } else if (pid == 0) {
        /* child */
        return fd;
    } else {
        /* parent */
        close(fd);
    }
    /* should never reach this */
    return FAILURE;
}

/*
 * connects and returns an ipv4 socket of any type on specified port
 * on address specified in in_addr ia4
 */

int connect_ipv4(int type, unsigned short port, struct in_addr * ia4)
{
    int fd = -1;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, (void *) ia4, sizeof(addr.sin_addr));

    fd = socket(AF_INET, type, 0);
    if (fd == -1) {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        Log("Unable to connect on port %d: %s\n", port, strerror(errno));
        close(fd);
        return FAILURE;
    }
    /* set non-blocking AFTER connect() call to avoid EINPROGRESS error */
    set_nonblock(fd);

    return fd;
}

/*
 * connects and returns an ipv6 socket of any type on specified port
 * on address specified in in6_addr ia6
 */

int connect_ipv6(int type, unsigned short port, struct in6_addr * ia6)
{
    int fd = -1;
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    memcpy(&addr.sin6_addr, (void *) ia6, sizeof(addr.sin6_addr));

    fd = socket(AF_INET6, type, 0);
    if (fd == -1) {
        Log("Unable to create socket\n");
        return FAILURE;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        Log("Unable to connect on port %d: %s\n", port, strerror(errno));
        close(fd);
        return FAILURE;
    }
    /* set non-blocking AFTER connect() call to avoid EINPROGRESS error */
    set_nonblock(fd);

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
    if (0 != listen(server_fd, 20)) {
        return FAILURE;
    }
    if (address_family == AF_INET) {
        len = sizeof(client_addr4);
        client_fd = accept(server_fd, (struct sockaddr *) &client_addr4, &len);
    } else if (address_family == AF_INET6) {
        len = sizeof(client_addr6);
        client_fd = accept(server_fd, (struct sockaddr *) &client_addr6, &len);
    } else {
        return FAILURE;
    }
    set_nonblock(client_fd);
    return client_fd;
}

/*
 * accepts new UDP connection from the fd specified, socked should be bound.
 * the existence of this function might seem mildly retarted. using TCP-like
 * behavior with UDP is necessary since a general bi-directional "UDP pipe"
 * (i.e., doesn't "care" who the client is) would be unable to return data
 * received from the remote service.  this function will wait til a datagram
 * is received, peek at the client address, and connect() the local socket
 * to the remote client, ensuring that data later returned from the remote
 * server can find its way back to the correct client.
 */

int accept_udp_connection(int client_fd, int address_family)
{
    struct sockaddr_in client_addr4;
    struct sockaddr_in6 client_addr6;
    socklen_t len = 0;
    int num_bytes;
    char recv_buf[2];

    memset(&client_addr4, 0, sizeof(client_addr4));
    memset(&client_addr6, 0, sizeof(client_addr6));
    if (address_family == AF_INET) {
        len = sizeof(client_addr4);
        /* this call to recvfrom() WILL block until we receive a UDP datagram.
         * since the MSG_PEEK flag is used, the data will not be removed from
         * the queue, which will allow us to read() it and forward later on.
         * we're essential replicating accept() from the TCP side of things.
         * this will give us client_addr, which allows us to connect()
         * the socket.
         */
        num_bytes = recvfrom(client_fd, &recv_buf, sizeof(recv_buf), MSG_PEEK,
                             (struct sockaddr *) &client_addr4, &len);
        if (num_bytes < 0) {
            Log("Error with recvfrom() in accept_udp_connection(): %s\n", strerror(errno));
            return FAILURE;
        }
        if (connect(client_fd, (struct sockaddr *)&client_addr4, sizeof(client_addr4)) != 0) {
            Log("Unable to connect() UDP socket to client: %s\n", strerror(errno));
            close(client_fd);
            return FAILURE;
        }
    } else if (address_family == AF_INET6) {
        len = sizeof(client_addr6);
        /* same as above, but IPv6 this time. */
        num_bytes = recvfrom(client_fd, &recv_buf, sizeof(recv_buf), MSG_PEEK,
                             (struct sockaddr *) &client_addr6, &len);
        if (num_bytes < 0) {
            Log("Error with recvfrom() in accept_udp_connection(): %s\n", strerror(errno));
            return FAILURE;
        }
        if (connect(client_fd, (struct sockaddr *)&client_addr6, sizeof(client_addr6)) != 0) {
            Log("Unable to connect() UDP socket to client: %s\n", strerror(errno));
            close(client_fd);
            return FAILURE;
        }
    } else {
        return FAILURE;
    }
    set_nonblock(client_fd);
    return client_fd;
}

/* this function buils a singlely linked list of interface ipv4/6 addresses 
 * it populates the global pinterface_ip_list if_list as the head of the list
 */

int get_host_ip_addresses()
{
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    pinterface_ip_list tmp, tmp2;
    char buf[64];

    if (getifaddrs(&myaddrs) != 0) {
        return FAILURE;
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        memset(buf, 0, sizeof(buf));

        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
            in_addr = &s4->sin_addr;
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            in_addr = &s6->sin6_addr;
        } else {
            continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf) - 1)) {
            fprintf(stderr, "%s: inet_ntop failed!\n", ifa->ifa_name);
            continue;
        }

        tmp = (pinterface_ip_list) calloc(sizeof(interface_ip_list), 1);
        if (tmp == NULL) {
            fprintf(stderr, "error allocating memeory\n");
            return FAILURE;
        }

        strncpy(tmp->name, ifa->ifa_name, sizeof(tmp->name) - 1);
        strncpy(tmp->addr, buf, sizeof(tmp->addr) - 1);
        tmp->type = ifa->ifa_addr->sa_family;

        if (tmp->type == AF_INET) {
            memcpy(&tmp->ia4, in_addr, sizeof(tmp->ia4));
        }
        if (tmp->type == AF_INET6) {
            memcpy(&tmp->ia6, in_addr, sizeof(tmp->ia6));
        }
        if (if_list == NULL) {
            if_list = tmp;
        } else {
            tmp2 = (pinterface_ip_list) if_list;
            while (tmp2->next != NULL) {
                tmp2 = (pinterface_ip_list) tmp2->next;
            }
            tmp2->next = (void *) tmp;
        }
    }

    freeifaddrs(myaddrs);
    return SUCCESS;
}

int disallow_socketcall(void) 
{
    struct sock_filter filter[] = {
        /* Validate architecture. */
        VALIDATE_ARCHITECTURE,

        /* Grab the system call number */
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_nr),

        /* Kill all socketcall, except PF_LOCAL*/
#if   defined(__x86_64__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socket, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) PF_LOCAL, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#elif defined(__arm__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socket, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) PF_LOCAL, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socketcall, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) SYS_socketcall_socket, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#elif defined(__i386__)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_socketcall, 0, 3),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, syscall_arg(0)),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, (unsigned long) SYS_socketcall_socket, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
#endif

        /* Allow everything else */
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW)
    };
    struct sock_fprog   prog = {
        .len    = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };


    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0, 0))
    {
        return 1;
    }

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, (unsigned long) &prog, 0, 0, 0))
    {
        return 1;
    }

    return 0;
}
