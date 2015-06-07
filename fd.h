typedef struct
{
    int     source;                 //!< Source file descriptor
    int     sink;                   //!< Sink file descriptor
} proxied_fd;

extern proxied_fd std_in, std_out;

void
set_cloexec_on_all_fds
(
);

int
dup_random
(
    int fd
);
