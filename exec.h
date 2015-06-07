typedef struct
{
    int     source;                 //!< Source file descriptor
    int     sink;                   //!< Sink file descriptor
} proxied_fd;

extern proxied_fd std_in, std_out;

int
create_pipes_fork_and_exec
(
    char **argv
);