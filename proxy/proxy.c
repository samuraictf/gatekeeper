#include "proxy.h"

static int max(int a, int b) { return a>b?a:b; }

typedef struct {
    int fd;
    callback_fn function;
    void *ctx;
}  registered_callback;

const size_t MAX_CALLBACKS = 64;

int child_fd;

registered_callback stdin_callbacks[MAX_CALLBACKS+1] = {0};
registered_callback stdout_callbacks[MAX_CALLBACKS+1] = {0};
registered_callback stderr_callbacks[MAX_CALLBACKS+1] = {0};

registered_callback *callbacks[] = {
    [STDIN_FILENO] = stdin_callbacks,
    [STDOUT_FILENO] = stdout_callbacks,
    [STDERR_FILENO] = stderr_callbacks
};

void ignore_SIGPIPE(int dontcare) {dontcare=0;};

/**
 * Fork, exec, and start pumping data.
 */
void pump_execvp(char** argv)
{
    signal(SIGPIPE, ignore_SIGPIPE);

    int pipes[2];
    int std_in_read, std_in_write;
    int std_out_read, std_out_write;
    int std_err_read, std_err_write;

    pipe2(pipes, O_CLOEXEC);
    std_in_read     = pipes[0];
    std_in_write    = pipes[1];

    pipe2(pipes, O_CLOEXEC);
    std_out_read    = pipes[0];
    std_out_write   = pipes[1];

    pipe2(pipes, O_CLOEXEC);
    std_err_read    = pipes[0];
    std_err_write   = pipes[1];

    child_fd = fork();

    if(child_fd == 0) {
        dup2(std_in_read,  STDIN_FILENO);
        dup2(std_out_write, STDOUT_FILENO);
        dup2(std_err_write, STDERR_FILENO);
        execvp(argv[0], &argv[0]);
        exit(-1);
    }

    //
    // Close the handles we don't need.  If these are left open,
    // then we won't get POLLHUP or recv() < 0 when the child dies,
    // since the other end of the pipe is still open somewhere.
    //
    close(std_in_read);
    close(std_out_write);
    close(std_err_write);

    //
    // The main loop
    //

    // These are the file descriptor source/sink pairs.
    int sources[] = { STDIN_FILENO, std_out_read, std_err_read };
    int sinks[] = { std_in_write, STDOUT_FILENO, STDERR_FILENO };
    int i = 0;

    // Buffer management.
    size_t buffer_used = 0;
    size_t buffer_allocated = 4096;
    char * buffer = malloc(buffer_allocated);


    while (1)
    {
        //
        // Find out which file descriptors have data, or not.
        //
        struct pollfd pollfds[3];
        int good_fds = 0;
        memset(pollfds, 0, sizeof(pollfds));
        for(i = 0; i < 3; i++) {
            pollfds[i].fd = sources[i];

            if(pollfds[i].fd >= 0) {
                pollfds[i].events = POLLIN;
                good_fds++;
            }
        }

        if(good_fds == 0)
        {
            // Everything closed nicely.
            return;
        }

        if (poll(pollfds, 3, -1) <= 0)
        {
            // An error occurred while polling.
            return;
        }


        //
        // Find out which file descriptor had an event.
        //
        for(i = 0; i < 3; i++) {
READLOOP:;
            int events = pollfds[i].revents;
            int source = sources[i];
            int sink   = sinks[i];
            size_t bytes_read = 0;


            // Data is available.  Save the 'source' and 'sink'.
            if(events & POLLIN) {

                // Data is available to be read, processed, and forwarded.
                bytes_read = read(source, buffer, buffer_allocated);

                // On failure, mark everything so that we don't try to continue
                // receiving data from it.
                if(bytes_read <= 0) {
                    close(source);

                    // Don't close our own stderr, which are necessary
                    // for diagnostics
                    if(sink != STDERR_FILENO)
                        close(sink);

                    sources[i] = -1;
                    continue;
                }

                buffer_used = bytes_read;

                //
                // Invoke all of the callbacks for this file descriptor.
                //
                registered_callback* p_callback = callbacks[i];

                while(p_callback && p_callback->function) {
                    callback_rv rv = p_callback->function(p_callback->ctx,
                                                         &buffer,
                                                         &buffer_used,
                                                         &buffer_allocated);

                    // A callback failed.  Be safe, shut down.
                    if(rv != CB_OKAY) {
                        return;
                    }

                    p_callback++;
                }

                //
                // Loop until all data is written to the sink socket.
                //
                ssize_t n_written = 0;
                for(size_t position = 0;
                    buffer_used > 0;
                    buffer_used -= n_written,
                    position += n_written)
                {

                    n_written = write(sink, &buffer[position], buffer_used);

                    // A write failed.  This means that either the child is
                    // dead / closed its STDIN, or our own STDOUT is closed.
                    //
                    // In either case, we should die.
                    if (n_written < 0)
                    {
                        return;
                    }
                }
            }

            //
            // One of the socket closed.  This can be delivered
            // simultaneously with POLLIN (i.e. data arrived, then)
            // the socket closed.
            //
            // We can't be sure that the above, single, call to recv()
            // completely emptied any buffered data.
            //
            // So we go back to the top of the loop and keep recv()ing
            // data until recv() fails.
            //
            if(events & POLLHUP) {
                if(events & POLLIN) {
                    goto READLOOP;
                } else {
                    close(source);

                    // Don't close our own stderr, which are necessary
                    // for diagnostics
                    if(sink != STDERR_FILENO)
                        close(sink);

                    sources[i] = -1;
                }
            }

        } // for each fd
    } // while(1)
}

void register_io_callback(int fd, callback_fn function, void* ctx)
{
    int i = 0;

    if(fd < 0 || fd > 2) {
        dprintf(2, "invalid fd: %i\n", fd);
    }

    registered_callback* cb = callbacks[fd];
    for(i = 0; cb->function != NULL; cb++, i++) {
        if(i >= MAX_CALLBACKS) {
            dprintf(2, "max callbacks reached\n");
            exit(-1);
        }
    }

    cb->function = function;
    cb->ctx      = ctx;
}
