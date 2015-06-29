#include "gatekeeper.h"

int log_fd;
struct in_addr log_addr;
int debugging;
pinterface_ip_list if_list;

/*
 * Prints usage and returns. Does not exit.
 */

void usage(void)
{
    printf("Usage:\n");
    printf("      -l <listen string> GK_LISTENSTR\n");
    printf("      -r <redirect string> GK_REDIRECTSTR\n");
    printf("      -o <optional log server string> GK_LOGSRV\n");
    printf("      -a <optional alarm in seconds> GK_ALARM\n");
    printf("      -d optional flag to limit disk access with RLIMIT_FSIZE 0 and ulimit 0777 GK_RLIMIT\n");
    printf("      -f optional flag to randomize file descriptors by opening /dev/urandom several times GK_RANDFD\n");
    printf("      -e optional flag randomize child environment GK_RANDENV\n");
    printf("      -t <optional chroot to this directory> GK_CHROOT\n");
    printf("      -k <optional filename to leak open fd on 1337 to forked child process> GK_LEAK_FD\n");
    printf("      -c <optional capture host string> GK_CAPHOST\n");
    printf("      -m <optional number for action to take on pcre match>\n");
    printf("         1: exit 2: random 3: evil\n");
    printf("      -D optional output extra debugging information\n");
    printf("      -n optional flag to block socket calles with seccomp\n");
    printf("         (WARNING: using -D without -o will send debugging info to stderr.  NEVER\n");
    printf("         do this in a live environment) GK_DEBUG\n");
    printf("      -I <optional path to text file of regexs to match INPUT traffic against> GK_INPUT_REGEXFILE\n");
    printf("      -O <optional path to text file of regexs to match OUTPUT traffic against> GK_OUTPUT_REGEXFILE\n");
    printf("      -b <optional path to text file of IP's to blacklist> GK_BLACKLIST_IP\n");
    printf("      -s <unimplemented skynet / machine learning cutoff>\n");
    printf("   example usage: gatekeeper -l stdio -r stdio:/home/atmail/atmail -o 10.0.0.2:2090\n");
    printf("                  gatekeeper -l tcpipv4:0.0.0.0:1234 -r stdio:/home/atmail/atmail -a 5\n");
    printf("                  gatekeeper -l tcpipv6:[::]:1234 -r stdio:/home/atmail/atmail\n\n");
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
 *
 */

int main(int argc, char * argv[])
{
    char * listenstr            = NULL;
    char * redirectstr          = NULL;
    char * logsrvstr            = NULL;
    char * capsrvstr            = NULL;
    char * alarmvalstr          = NULL;
    char * chrootstr            = NULL;
    char * randenvstr           = NULL;
    char * rlimitstr            = NULL;
    char * debugstr             = NULL;
    char * randfdstr            = NULL;
    char * regex_input_fname    = NULL;
    char * regex_output_fname   = NULL;
    char * blacklist_ip_fname   = NULL;
    char * leak_fd_fname        = NULL;
    char * blocksockstr         = NULL;
    char * match_actionstr      = NULL;
    int c                       = -1;
    int f                       = -1;
    int randenv                 = 0;
    int randfd                  = 0;
    int rlimit                  = 0;
    int blocksock               = 0;
    int match_action            = 1; // exit
    unsigned int i              = 0;
    unsigned int alarmval       = 0;
    unsigned int seed           = 0;
    int listen_fd_r             = -1;
    int listen_fd_w             = -1;
    int remote_fd_r             = -1;
    int remote_fd_w             = -1;
    int highest_fd              = -1;
    int leak_fd                 = -1;
    ringbuffer_t * in_ringbuf   = NULL;
    ringbuffer_t * out_ringbuf  = NULL;
    pcre_list_t * in_pcre_list  = NULL;
    pcre_list_t * out_pcre_list = NULL;
    struct rlimit rl;
    fd_set readfds;
    Skynet*  skynet             = NULL;

    /* initialize globals*/
    debugging = 0;
    if_list = NULL;

    memset(&log_addr, 0, sizeof(log_addr));
    memset(&rl, 0, sizeof(rl));

    /* capture environment configurations */
    listenstr           = getenv("GK_LISTENSTR");
    redirectstr         = getenv("GK_REDIRECTSTR");
    logsrvstr           = getenv("GK_LOGSRV");
    capsrvstr           = getenv("GK_CAPHOST");
    alarmvalstr         = getenv("GK_ALARM");
    chrootstr           = getenv("GK_CHROOT");
    randenvstr          = getenv("GK_RANDENV");
    rlimitstr           = getenv("GK_RLIMIT");
    debugstr            = getenv("GK_DEBUG");
    randfdstr           = getenv("GK_RANDFD");
    blocksockstr        = getenv("GK_BLOCKSOCK");
    regex_input_fname   = getenv("GK_INPUT_REGEXFILE");
    regex_output_fname  = getenv("GK_OUTPUT_REGEXFILE");
    blacklist_ip_fname  = getenv("GK_BLACKLIST_IP");
    match_actionstr     = getenv("GK_MATCH_ACTION");
    leak_fd_fname       = getenv("GK_LEAK_FD");

    /* at least 5 arguments to run: program, -l, listenstr, -r, redirstr
     * unless we have at least listenstr and redirectstr from env
     */
    if (listenstr == NULL || redirectstr == NULL) {
        if (argc < 5) {
            usage();
            goto cleanup;
        }
    }

    /* setup RNG */
    f = open("/dev/urandom", O_RDONLY, NULL);
    if (f == -1 || sizeof(seed) != read(f, &seed, sizeof(seed))) {
        Log("RNG initialized with time\n");
        seed = time(0) + getpid();
    }
    srand(seed);
    if (f != -1) {
        close(f);
        f = -1;
    }
    seed = 0;

    /* avoid zombie children */
    if (signal(SIGCHLD, sigchld) == SIG_ERR) {
        Log("Unable to set SIGCHLD handler\n");
        goto cleanup;
    }

    /* alarm parse */
    if (alarmvalstr != NULL) {
        alarmval = atoi(alarmvalstr);
    }

    if (match_actionstr != NULL) {
        match_action = atoi(match_actionstr);
    }

    /* flags parse */
    if (randenvstr != NULL) {
        randenv = 1;
    }

    if (debugstr != NULL) {
        debugging = 1;
    }

    if (randfdstr != NULL) {
        randfd = 1;
    }

    if (rlimitstr != NULL) {
        rlimit = 1;
    }

    if (blocksockstr != NULL) {
        blocksock = 1;
    }
    /* cmdline parse, environment configurations take precident */
    while ((c = getopt(argc, argv, "hDt:I:O:l:r:k:o:c:a:dfeb:s:nm:")) != -1) {
        switch (c) {
        case 'h':
            /* help and exit*/
            usage();
            goto cleanup;
        case 'l':
            /* save off listen argument string -- required*/
            if (listenstr == NULL) {
                listenstr = optarg;
            }
            break;
        case 'r':
            /* save off redirect argument string -- required*/
            if (redirectstr == NULL) {
                redirectstr = optarg;
            }
            break;
        case 'o':
            /* logging server string of where to send Log() function data */
            if (logsrvstr == NULL) {
                logsrvstr = optarg;
            }
            break;
        case 'c':
            /* capture server string of where to send intercepted data */
            if (capsrvstr == NULL) {
                capsrvstr = optarg;
            }
            break;
        case 'a':
            /* alarm string for alarm() call, convert to int val */
            if (alarmvalstr == NULL) {
                alarmval = atoi(optarg);
            }
            break;
        case 'e':
            /* flag to enable randomized envp on fork/exec */
            if (randenvstr == NULL) {
                randenv = 1;
            }
            break;
        case 'D':
            /* Give me all the output */
            if (debugstr == NULL) {
                debugging = 1;
            }
            break;
        case 'd':
            /* flag, to block disk writes with rlimit and umask calls */
            if (rlimitstr == NULL) {
                rlimit = 1;
            }
            break;
        case 't':
            /* we don't need root to chroot if we unshare the user namespace */
            if (chrootstr == NULL) {
                chrootstr = optarg;
            }
            break;
        case 'f':
            /* this will cause later opens after exec to have unexpected fd numbers, we don't close these */
            if (randfdstr == NULL) {
                randfd = 1;
            }
            break;
        case 'I':
            /* file name for input pcre filterng */
            if (regex_input_fname == NULL) {
                regex_input_fname = optarg;
            }
            break;
        case 'O':
            /* file name for output pcre filterng */
            if (regex_output_fname == NULL) {
                regex_output_fname = optarg;
            }
            break;
        case 'b':
            /* filename for blacklisted IP's for filtering */
            if (blacklist_ip_fname == NULL) {
                blacklist_ip_fname = optarg;
            }
            break;
        case 'k':
            /* filename for fd leak */
            if (leak_fd_fname == NULL) {
                leak_fd_fname = optarg;
            }
            break;
        case 'n':
            if (blocksockstr == NULL) {
                blocksock = 1;
            }
            break;
        case 'm':
            if (match_actionstr == NULL) {
                match_actionstr = optarg;
            }
            break;
        case 's':
        {
            double cutoff = atof(optarg);
            skynet = Skynet_new(cutoff);
            break;
        }
        case '?':
            /* help */
            usage();
            if ('l' == optopt || 'r' == optopt || 'k' == optopt || 'o' == optopt || 'c' == optopt || 'a' == optopt || 'I' == optopt || 'O' == optopt || 'b' == optopt || 'm' == optopt)
                Log("Option '-%c' requires an argument\n", optopt);
            else if (isprint(optopt))
                Log("Unknown options '-%c'\n", optopt);
            else
                Log("Unknown getopt return: 0x%x\n", c);
            goto cleanup;
        }
    }

    /* check for required arguments */
    if (listenstr == NULL) {
        Log("Argument -l (listen string) is required GK_LISTENSTR\n");
        goto cleanup;
    }

    if (redirectstr == NULL) {
        Log("Argument -r (redirect string) is required GK_REDIRECTSTR\n");
        goto cleanup;
    }

    /* if a regex input file was specified, parse it */
    if (regex_input_fname != NULL) {
        if ((in_pcre_list = parse_pcre_inputs(regex_input_fname)) == NULL) {
            Log("Failed to parse all pcre input filters, bailing out...\n");
            goto cleanup;
        }
    }
    /* if a regex output file was specified, parse it */
    if (regex_output_fname != NULL) {
        if ((out_pcre_list = parse_pcre_inputs(regex_output_fname)) == NULL) {
            Log("Failed to parse all pcre output filters, bailing out...\n");
            goto cleanup;
        }
    }

    if (in_pcre_list != NULL) {
        in_ringbuf = ringbuffer_create(RECVBUF_SIZE);
        if (in_ringbuf == NULL) {
            Log("Error creating input ring buffer\n");
            goto cleanup;
        }
    }
    
    if (out_pcre_list != NULL) {
        out_ringbuf = ringbuffer_create(RECVBUF_SIZE);
        if (out_ringbuf == NULL) {
            Log("Error creating output ring buffer\n");
            goto cleanup;
        }
    }

    /* setup logging server globals */
    if (logsrvstr != NULL) {
        if (setup_logsocket(logsrvstr) == FAILURE) {
            Log("Failed to setup log server socket\n");
            goto cleanup;
        }
    }

    /* open many file descriptors to "randomize" the real i/o fd's used by
       forked children */
    if (randfd == 1) {
        for (i = 0; i < (unsigned int) ((rand() % 500) + 256); i++) {
            open("/dev/urandom", O_RDONLY, NULL);
        }
    }

    /* setting RLIMIT_FSIZE to 0 will make write calls after exec fail with SIGXFSZ
       umask will render files unusable without a chmod first */
    if (rlimit == 1) {
        umask(0777);
        setrlimit(RLIMIT_FSIZE, &rl);
    }


    if (leak_fd_fname != NULL) {
        leak_fd = open(leak_fd_fname, O_RDONLY, NULL);
        dup2(leak_fd, 1337);
        close(leak_fd);
    }

    if (blacklist_ip_fname != NULL) {
        parse_blacklist_file(blacklist_ip_fname);
    }

    /* chroot! */
    if (chrootstr != NULL) {
        unshare(CLONE_NEWUSER);
        chroot(chrootstr);
        chdir(chrootstr);
    }

    /* setup local listener fd's, will block to accept connections */
    if (setup_connection(listenstr, &listen_fd_r, &listen_fd_w, LOCAL) == FAILURE) {
        goto cleanup;
    }

    /* block socket calls, needs to happen before exec, won't work with remote connections */
    if (blocksock != 0) {
        disallow_socketcall();
    }
    
    if(blacklist_ip_fname != NULL) {
        if (blacklist_check_fd(listen_fd_r) == 1) {
            socklen_t len = sizeof(struct sockaddr);
            struct sockaddr addr;
            memset(&addr, 0, sizeof(addr));
            char myaddr[65];
            memset (myaddr, 0, sizeof(myaddr));
            getpeername(0, &addr, &len);
            inet_ntop(addr.sa_family, &addr, myaddr, sizeof(myaddr)-1);
            Log("Blacklist connection from: %s\n", myaddr);
            exit(0);
        }
    }
    /* setup remote fd's */
    if (setup_connection(redirectstr, &remote_fd_r, &remote_fd_w, randenv ? REMOTE_RAND_ENV : REMOTE) == FAILURE) {
        goto cleanup;
    }
    
    /* set alarm if one was specified, must be after connection establised to be in
       correct fork-ed process */
    if (alarmval != 0) {
        alarm(alarmval);
    }

    /* check for IP blacklist */

    /* start the pumps */
    if (listen_fd_r > remote_fd_r) {
        highest_fd = listen_fd_r;
    } else {
        highest_fd = remote_fd_r;
    }

    /*  listen_fd_r -> remote_fd_w
        remote_fd_r -> listen_fd_w */

    while (1) {
        /* move out of loop and memcpy from tmp variable instead? maybe later. */
        FD_ZERO(&readfds);
        FD_SET(listen_fd_r, &readfds);
        FD_SET(remote_fd_r, &readfds);
        if (select(highest_fd + 1, &readfds, NULL, NULL, NULL) == -1) {
            Log("select() threw an error: %s\n", strerror(errno));
            goto cleanup;
        }
        /* read in data from the socket that is ready */
        if (FD_ISSET(listen_fd_r, &readfds)) {
            if (proxy_packet(listen_fd_r, remote_fd_w, in_pcre_list, in_ringbuf, NULL ) < 0) {
                goto cleanup;
            }
        }
        if (FD_ISSET(remote_fd_r, &readfds)) {
            if (proxy_packet(remote_fd_r, listen_fd_w, out_pcre_list, out_ringbuf, skynet ) < 0) {
                goto cleanup;
            }
        }
    }


cleanup:
    if (listen_fd_r != -1)
        close(listen_fd_r);
    if (listen_fd_w != -1)
        close(listen_fd_w);
    if (remote_fd_r != -1)
        close(remote_fd_r);
    if (remote_fd_w != -1)
        close(remote_fd_w);
    if (log_fd != -1)
        close(log_fd);
    if (in_ringbuf)
        ringbuffer_free(in_ringbuf);
    if (out_ringbuf)
        ringbuffer_free(out_ringbuf);
    free_list(in_pcre_list);
    free_list(out_pcre_list);
    Skynet_delete(skynet);
    return SUCCESS;
}

int proxy_packet(int socket_src, int socket_dst, struct pcre_list *filters, ringbuffer_t *ring_buffer, Skynet* skynet )
{
    int num_bytes = 0;
    int ringbuf_cnt = 0;
    char recvbuf[RECVBUF_SIZE] = {0};
    char pcrebuf[RECVBUF_SIZE] = {0};
    num_bytes = read(socket_src, recvbuf, RECVBUF_SIZE);
    if(!num_bytes) {
        if (debugging) { Log("Got EOF on socket %d\n",socket_src); }
        return -1;
    }
    if(filters) {
        if(ringbuffer_write(ring_buffer, (uint8_t *)recvbuf, num_bytes) != (unsigned int)num_bytes) {
            Log("Weird, could not write all received data to the input ring buffer.  Bailing out...\n");
            return -1;
        }
        ringbuf_cnt = ringbuffer_peek(ring_buffer, (uint8_t *)pcrebuf, RECVBUF_SIZE);
        if (check_for_match(filters, pcrebuf, ringbuf_cnt) != 0) {
            /* we found a match.  now what do we do?  die?  flip bits? */
            Log("Blacklist filter match; dropping connection\n");
            return -1;
        }
    }
    if( skynet )
    {
        Skynet_processPacket( skynet, socket_src, recvbuf, num_bytes );
    }

    if (write(socket_dst, recvbuf, num_bytes) != num_bytes) {
        Log("Huh? Couldn't write all available data to remote_fd...\n");
        /* fail here? */
    }
    return num_bytes;
}

/*
 * This farms exit status from forked children to avoid
 * having any zombie processes lying around
 */
void sigchld()
{
    int status;
    while (wait4(-1, &status, WNOHANG, NULL) > 0) {  }
}

/*
 * Setups a socket to be used with the logging server. IPv4 only. UDP socket.
 */
int setup_logsocket(char * logsrvstr)
{
    char * addr_start = NULL;
    char * port_start = NULL;
    unsigned short port = 0;
    
    /* parse logsrvstr manually */
    if (logsrvstr == NULL) {
        return FAILURE;
    }
    addr_start = logsrvstr;
    port_start = strchr(addr_start, ':');
    if (port_start == NULL) {
        return FAILURE;
    }
    *port_start = '\x00';
    port_start++;
    if (inet_pton(AF_INET, addr_start, &log_addr) != 1) {
        Log("Invalid IP address specified in address string.\n");
        return FAILURE;
    }
    port = (unsigned short)atoi(port_start);
    log_fd = connect_ipv4(SOCK_DGRAM, port, &log_addr);
    if (log_fd == -1) {
        return FAILURE;
    }
    /* now that the log socket is connected, we can use read()/write() */
        
    return SUCCESS;
}

/*
 * Takes in the instr and returns two fd's, one for output one for input
 * in all socket type connections the fd's are the same value.
 * In the case of stdin these are stdin and stdout and stderr is dup2() to stdout.
 *
 * if local is 1 then it will listen for local connections
 * if local is 2 then it will listen for local connections AND randomize the
 *   environment before execve
 * if local is 0 it will connect or exec to the remote target
 *
 * At the end of this function server sockets will have a connected socket.
 */
int setup_connection(char * instr, int * out_fd_r, int * out_fd_w, int local)
{
    struct in_addr addr;
    struct in6_addr addr6;
    unsigned short port = 0;
    char str[INET6_ADDRSTRLEN];
    int fd_r = -1;
    int fd_w = -1;
    int in_pipe[2];     /* gatekeeper -> remote proc */
    int out_pipe[2];    /* remote proc -> gatekeeper */
    pid_t pid = 0;
    char * args_start = NULL;
    unsigned int arg_count = 0;
    unsigned int i = 0;
    char * tmp = NULL;
    char * last_arg = NULL;
    char ** envp = NULL;
    char ** argv = NULL;

    *out_fd_r = fd_r;
    *out_fd_r = fd_w;

    memset(str, 0, sizeof(str));
    memset(&addr, 0, sizeof(addr));
    memset(&addr6, 0, sizeof(addr6));

    if (strncmp("stdio", instr, strlen("stdio")) == 0) {
        if (local == LOCAL) {
            /* remove buffering on stdio */
            if (0 != setvbuf(stdin, (char*) NULL, _IONBF, 0) ||
                    0 != setvbuf(stdout, (char*) NULL, _IONBF, 0) ||
                    0 != setvbuf(stderr, (char*) NULL, _IONBF, 0)) {
                return FAILURE;
            }
            fd_r = 0;
            fd_w = 1;
            /* redirect stderr to stdout so we only have 2 fd's to deal with */
            dup2(1, 2);
        } else if (local == REMOTE || local == REMOTE_RAND_ENV) {
            /* set up pipes. pipe names are from the child (remote) process perspective */
            pipe(in_pipe);
            pipe(out_pipe);

            pid = fork();
            if (pid == -1) {
                /* fork() failed.  this can happen. */
                Log("fork() failed: %s\n", strerror(errno));
                return FAILURE;
            }
            if (pid == 0) {
                /* child process */
                args_start = (strchr(instr, ':') + 1);
                if (args_start == NULL && *args_start++ != '\0') {
                    Log("Invalid remote string, stdio specified without a ':' followed by a program to run. i.e. \"stdio:/bin/cat foo\"\n");
                    return FAILURE;
                }

                /* build up an argv */
                /* count the number of spaces for arguments */
                for (arg_count = 0; *instr; instr++) {
                    if (*instr == ' ') {
                        arg_count++;
                    }
                }

                /* create arg_count +2 for argv[0] and NULL termination */
                argv = calloc(sizeof(void *) * (arg_count + 2), 1);
                if (argv == NULL) {
                    return FAILURE;
                }

                last_arg = args_start;
                tmp = args_start;
                for (i = 0; i < arg_count + 1 && *tmp; tmp++) {
                    if (*tmp == ' ') {
                        *tmp = '\0';
                        argv[i] = last_arg;
                        last_arg = tmp + 1;
                        i++;
                    }
                }
                argv[i] = last_arg;

                /* clean up pipes by closing file descriptors we won't use */
                close(in_pipe[1]);
                close(out_pipe[0]);

                /* dup stdio to proper pipes */
                dup2(in_pipe[0], 0);
                dup2(out_pipe[1], 1);
                dup2(out_pipe[1], 2);

                if (local == REMOTE_RAND_ENV) {
                    envp = build_rand_envp();
                } 
                /* exec program using environment of parent for now. */
                if (debugging) {
                    Log("executing ");
                    for (i = 0; i <= arg_count; i++) {
                        Log("%s ", argv[i]);
                    }
                    Log("\n");
                }
                execve(argv[0], argv, envp);
                /* should never get here */
                Log("execve() of %s failed: %s\n", argv[0], strerror(errno));
                return FAILURE;
            }
            /* parent process */

            /* clean up pipes by closing file descriptors we won't use */
            close(in_pipe[0]);
            close(out_pipe[1]);

            /* r/w file descriptors are the parent end of the r/w pipes */
            fd_r = out_pipe[0];
            fd_w = in_pipe[1];
        }
    } else if (strncmp("tcpipv4", instr, strlen("tcpipv4")) == 0) {
        if (parse_address_string(instr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS) {
            Log("TCP %s:%d\n", inet_ntoa(addr), port);
        } else {
            return FAILURE;
        }
        if (local == LOCAL) {
            fd_r = fd_w = init_tcp4(port, &addr, NULL);
        } else if (local == REMOTE) {
            fd_r = fd_w = connect_ipv4(SOCK_STREAM, port, &addr);
        }
    } else if (strncmp("udpipv4", instr, strlen("udpipv4")) == 0) {
        if (parse_address_string(instr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS) {
            Log("UDP %s:%d\n", inet_ntoa(addr), port);
        } else {
            return FAILURE;
        }
        if (local == LOCAL) {
            fd_r = fd_w = init_udp4(port, &addr);
        } else if (local == REMOTE) {
            fd_r = fd_w = connect_ipv4(SOCK_DGRAM, port, &addr);
        }
    } else if (strncmp("tcpipv6", instr, strlen("tcpipv6")) == 0) {
        if (parse_address_string(instr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS) {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str) - 1);
            Log("TCP [%s]:%d\n", str, port);
        } else {
            return FAILURE;
        }
        if (local == LOCAL) {
            fd_r = fd_w = init_tcp6(port, &addr6, NULL);
        } else if (local == REMOTE) {
            fd_r = fd_w = connect_ipv6(SOCK_STREAM, port, &addr6);
        }
    } else if (strncmp("udpipv6", instr, strlen("udpipv6")) == 0) {
        if (parse_address_string(instr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS) {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str) - 1);
            Log("UDP [%s]:%d\n", str, port);
        } else {
            return FAILURE;
        }
        if (local == LOCAL) {
            fd_r = fd_w = init_udp6(port, &addr6);
        } else if (local == REMOTE) {
            fd_r = fd_w = connect_ipv6(SOCK_DGRAM, port, &addr6);
        }
    } else {
        Log("Invalid listen string, one of stdio, tcpipv4, tcpipv6, udpipv4, udpipv6 is required\n");
        return FAILURE;
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
 *     - udpipv4:192.168.0.1:1234          ...
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

    if (instr == NULL || port == NULL || addr == NULL || addr_size == 0 || address_family == 0) {
        Log("Missing parameters to parse_address_string.\n");
        return FAILURE;
    }

    if (address_family != AF_INET && address_family != AF_INET6) {
        Log("Invalid address family specified.\n");
        return FAILURE;
    }

    if ((addr_size != sizeof(struct in_addr) && address_family == AF_INET) ||
            (addr_size != sizeof(struct in6_addr) && address_family == AF_INET6)) {
        Log("Invalid address size specified.\n");
        return FAILURE;
    }

    /* advance past tcpipv4 or tcpipv6 we start after the ':' */
    string_start = strchr(instr, ':');
    if (string_start == NULL) {
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
    if (address_family == AF_INET6) {
        addr_start = strchr (string_start, '[');
        end = strchr (string_start, ']');
        if (addr_start == NULL && end == NULL) {
            /* only a port was specified */
            port_start = string_start;
            addr_start = NULL;
            memset(addr, 0, addr_size);
        } else if (addr_start != NULL && end != NULL) {
            /* remove the '[', ']', and ':' following the ']' character */
            *addr_start = '\x00';
            addr_start++;
            *end = '\x00';
            end++;
            if (*end != ':') {
                Log("No port specified with IPv6 address. Format as tcpipv6:[::aaaa:bbbb:cccc]:80\n");
                return FAILURE;
            }
            *end = '\x00';
            end++;
            port_start = end;
        } else {
            Log("Invalid IPv6 address specified in address string. Format as tcpipv6:[::aaaa:bbbb:cccc]:80\n");
            return FAILURE;
        }
    } else if (address_family == AF_INET) {
        port_start = strchr(string_start, ':');
        if (port_start != NULL) {
            addr_start = string_start;
            *port_start = '\x00';
            port_start++;
        } else {
            /* only a port was specified */
            port_start = string_start;
            addr_start = NULL;
            memset(addr, 0, addr_size);
        }
    }

    if (addr_start != NULL) {
        if (inet_pton(address_family, addr_start, addr) != 1) {
            Log("Invalid IP address specified in address string.\n");
            return FAILURE;
        }
    }

    tmp_port = atoi(port_start);
    if (tmp_port > 0 && tmp_port <= 0xffff) {
        *port = (unsigned short) tmp_port;
    } else {
        Log("Invalid port specification in address string.\n");
        return FAILURE;
    }
    return SUCCESS;
}

char ** build_rand_envp()
{
    char ** ret = NULL;
    int n = rand() % 1000 + 1;
    int i = 0;
    int x = 0;
    int name_len = 0;
    int val_len = 0;
    unsigned char c = 0;
    ret = calloc(sizeof(void*) * (n + 1), 1);
    if (ret == NULL)
        return ret;
    for (i = 0; i < n; i++) {
        name_len = rand() % 500 + 1;
        val_len = rand() % 500 + 1;
        ret[i] = calloc(name_len + val_len + 2, 1);
        if (ret[i] == NULL)
            break;
        for (x = 0; x < name_len; x++) {
            c = (unsigned char) rand();
            if (isalnum(c)) {
                ret[i][x] = c;
            } else {
                x--;
            }
        }
        ret[i][x] = '=';
        for (x = name_len + 1; x < name_len + val_len + 1; x++) {
            c = (unsigned char) rand() * 255;
            if (isalnum(c)) {
                ret[i][x] = c;
            } else {
                x--;
            }
        }
        ret[i][x] = '\0';
    }
    ret[i] = NULL;
    return ret;
}

