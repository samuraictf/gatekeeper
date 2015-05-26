#include "gatekeeper.h"

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
    Log("      -t <optional chroot to this directory> \n");
    Log("      -e <optional randomize child environment> \n");
    Log("      -d optional limit disk access with RLIMIT_FSIZE 0 and ulimit 0\n");
    Log("      -f optional randomize file descriptors by opening and not closing /dev/urandom several times\n");
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
    char * keyfile        = NULL;
    char * listenstr      = NULL;
    char * redirectstr    = NULL;
    char * logsrvstr      = NULL;
    char * capsrvstr      = NULL;
    int c                 = -1;
    int f                 = -1;
    int rand_env          = 0;
    unsigned int i        = 0;
    unsigned int alarmval = 0;
    unsigned int seed     = 0;
    int listen_fd_r       = -1;
    int listen_fd_w       = -1;
    int remote_fd_r       = -1;
    int remote_fd_w       = -1;
    struct rlimit rl;


    /* initialize globals*/
    log_fd = -1;

    memset(&log_addr, 0, sizeof(log_addr));
    memset(&rl, 0, sizeof(rl));

    /* at least 5 arguments to run: program, -l, listenstr, -r, redirstr */
    if (argc < 5)
    {
        usage();
        goto cleanup;
    }

    /* setup RNG */
    f = open("/dev/urandom", O_RDONLY, NULL);
	if (f == -1 || sizeof(seed) != read(f, &seed, sizeof(seed)))
	{
		seed=time(0);
	}
	srand(seed);
	if(f != -1)
	{
		close(f);
		f = -1;
	}
	seed = 0;
	build_rand_envp();
    /* avoid zombie children */
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
    {
        Log("Unable to set SIGCHLD handler");
        goto cleanup;
    }

    while ((c=getopt(argc, argv, "hl:r:k:o:c:a:t:dfe")) != -1)
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
        case 'e':
            rand_env = 1;
            break;
        case 'd':
            /* setting RLIMIT_FSIZE to 0 will make write calls after exec fail with SIGXFSZ
             * umask will render files unusable without a chmod first
             */
			umask(0);
			setrlimit(RLIMIT_FSIZE, &rl);
            break;
        case 't':
        	/* we don't need root to chroot if we unshare the user namespace */
        	unshare(CLONE_NEWUSER);
        	chroot(optarg);
        	break;
        case 'f':
            /* this will cause later opens after exec to have unexpected fd numbers, we don't close these */
			for (i = 0; i < (unsigned int) ((rand() % 100) + 1); i++)
			{
				open("/dev/urandom", O_RDONLY, NULL);
			}
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
        Log("Argument -l (listen string) is required\n");
        goto cleanup;
    }

    if (redirectstr == NULL)
    {
        Log("Argument -r (redirect string) is required\n");
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

    /* setup local listener fd's, will block to accept connections */
    if (setup_connection(listenstr, &listen_fd_r, &listen_fd_w, LOCAL) == FAILURE)
    {
        goto cleanup;
    }

    /* setup remote fd's */
    if (setup_connection(redirectstr, &remote_fd_r, &remote_fd_w, rand_env ? REMOTE_RAND_ENV : REMOTE) == FAILURE)
    {
    	goto cleanup;
    }

    /* start the pumps */


cleanup:
    if (listen_fd_r != -1)
        close(listen_fd_r);
    if (listen_fd_w != -1)
        close(listen_fd_w);
    if (remote_fd_r != -1)
        close(listen_fd_r);
    if (remote_fd_w != -1)
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
    pid_t pid = 0;
    char * args_start = NULL;
    unsigned int arg_count = 0;
    unsigned int i = 0;
    char * tmp = NULL;
    char * last_arg = NULL;
    char * arg = NULL;
    char ** envp = NULL;
    char ** argv = NULL;

    *out_fd_r = fd_r;
    *out_fd_r = fd_w;

    memset(str, 0, sizeof(str));
    memset(&addr, 0, sizeof(addr));
    memset(&addr6, 0, sizeof(addr6));

    if (strncmp("stdio", instr, strlen("stdio")) == 0)
    {
    	if (local==LOCAL)
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
    	else if (local == REMOTE || local == REMOTE_RAND_ENV)
    	{
    		pid = fork();
    		if (pid == 0)
    		{
    			/* child process */
    			args_start = strchr(instr, ':');
    			if (args_start == NULL && *args_start++ != '\0')
    			{
    				Log("Invalid remote string, stdio specified without a ':' followed by a program to run. i.e. \"stdio:/bin/cat foo\"\n");
    				return FAILURE;
    			}

    			/* build up an argv */
    			/* count the number of spaces for arguments */
    			for (arg_count = 0; *instr; instr++)
    			{
    				if (*instr == ' ')
    				{
    					arg_count++;
    				}
    			}

    			/* create arg_count +2 for argv[0] and NULL termination */
    			argv = calloc(sizeof(void *) * (arg_count + 2), 1);
    			if (argv==NULL)
    			{
    				return FAILURE;
    			}

    			last_arg = args_start;
    			tmp = args_start;
    			for (i=0; i<arg_count+1, *tmp; tmp++)
    			{
    				if (*tmp == ' ')
    				{
    					*tmp = '\0';
    					argv[i] = last_arg;
    					last_arg = tmp+1;
    					i++;
    				}
    			}
    			argv[i]=last_arg;

    			if (local == REMOTE_RAND_ENV)
    			{
    				envp = build_rand_envp();
    				/*argv = getenv("GATEKEEPER_EXEC_ARGS");
    				execve()*/
    			}
    		}
    	}
    }
    else if (strncmp("tcpipv4", instr, strlen("tcpipv4")) == 0)
    {
        if (parse_address_string(instr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("TCP %s:%d\n", inet_ntoa(addr), port);
        }
        else
        {
            return FAILURE;
        }
        if (local==LOCAL)
        {
        	fd_r = fd_w = init_tcp4(port, &addr, NULL);
        }
        else if (local == REMOTE)
        {

        }
    }
    else if (strncmp("udpipv4", instr, strlen("udpipv4")) == 0)
    {
        if (parse_address_string(instr, &addr, sizeof(addr), &port, AF_INET) == SUCCESS)
        {
            Log("UDP %s:%d\n", inet_ntoa(addr), port);
        }
        else
        {
            return FAILURE;
        }
        if(local==LOCAL)
        {
        	fd_r = fd_w = init_udp4(port, &addr);
        }
        else if (local == REMOTE)
        {

        }
    }
    else if (strncmp("tcpipv6", instr, strlen("tcpipv6")) == 0)
    {
        if (parse_address_string(instr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("TCP [%s]:%d\n", str, port);
        }
        else
        {
            return FAILURE;
        }
        if(local==LOCAL)
        {
        	fd_r = fd_w = init_tcp6(port, &addr6, NULL);
       	}
        else if (local == REMOTE)
        {

        }
    }
    else if (strncmp("udpipv6", instr, strlen("udpipv6")) == 0)
    {
        if (parse_address_string(instr, &addr6, sizeof(addr6), &port, AF_INET6) == SUCCESS)
        {
            inet_ntop(AF_INET6, (char *)&addr6, str, sizeof(str)-1);
            Log("UDP [%s]:%d\n", str, port);
        }
        else
        {
            return FAILURE;
        }
        if(local==LOCAL)
        {
        	fd_r = fd_w = init_udp6(port, &addr6);
        }
        else if (local == REMOTE)
        {

        }
    }
    else
    {
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

char ** build_rand_envp()
{
	char ** ret = NULL;
	int n = rand() % 1000 + 1;
	int i = 0;
	int x = 0;
	int name_len = 0;
	int val_len = 0;
	unsigned char c = 0;
	ret = calloc(sizeof(void*) * (n+1), 1);
	if (ret == NULL)
		return ret;
	for (i = 0; i < n; i++)
	{
		name_len = rand() % 500 + 1;
		val_len = rand() % 500 + 1;
		ret[i] = calloc(name_len + val_len + 2, 1);
		if (ret[i] == NULL)
			break;
		for (x = 0; x < name_len; x++)
		{
			c = (unsigned char) rand();
			if(isalnum(c))
			{
				ret[i][x] = c;
			}
			else
			{
				x--;
			}
		}
		ret[i][x] = '=';
		for (x = name_len + 1; x < name_len+val_len+1; x++)
		{
			c = (unsigned char) rand() * 255;
			if(isalnum(c))
			{
				ret[i][x] = c;
			}
			else
			{
				x--;
			}
		}
		ret[i][x] = '\0';
	}
	ret[i] = NULL;
	return ret;
}

