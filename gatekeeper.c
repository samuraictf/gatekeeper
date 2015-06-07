       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
       #include <unistd.h>


int real_stdin, real_stdout, real_stderr;
int proxy_stdin[2], proxy_stdout[2], proxy_stderr[2];


void
initialize_random
(
)
{
    int seed;
    int urandom = open("/dev/urandom", O_RDONLY);
    read(urandom, &seed, sizeof(seed));
    srand(seed);
}



void
create_proxy_fds
(
)
{
    real_stdin  = dup2(0);
    real_stdout = dup2(1);
    real_stderr = dup2(2);

    pipe(proxy_stdin);
    pipe(proxy_stdout);
    pipe(proxy_stderr);

    dup2(0, proxy_stdin[0]);
    dup2(1, proxy_stdout[0]);
    dup2(2, proxy_stderr[0]);

    close(proxy_stdin[0]);
    close(proxy_stdout[0]);
    close(proxy_stderr[0]);
}



typedef void (*funcptr)
(
);

funcptr initializers[] = {
    initialize_random,
    create_proxy_fds,
    initialize_breakpad,
    set_cloexec_on_all_fds,
}



__attribute__((constructor))
static void
main
(
    int     argc,
    char    ** argv,
    char    **envp
)
{
    for (auto &initializer: initializers)
    {
        initializer();
    }
}
