#pragma once
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <linux/sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <pcre.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

/**
 * Callback return values.
 */
typedef enum {
    CB_ERROR=-1,
    CB_OKAY=0
} callback_rv;

/**
 * I/O callback hook function type.
 *
 * @param  ctx
 * Pointer to the structure passed to register_io_callback
 * as the ctx parameter.
 *
 * @param  buf
 * Buffer of data which was received.
 *
 * @param  buf_used
 * Number of bytes which were received.
 *
 * @param  buf_allocated_size
 * Allocated size of buf_size.
 *
 * @return
 * One of the return codes in callback_rv.
 *
 * @notes
 * buf, buf_used, and buf_allocated_size may all be modified by
 * the callback.  buf should only be resized with realloc().
 */
typedef callback_rv (*callback_fn)(void* ctx,
                                   void**  buf,
                                   size_t* buf_used,
                                   size_t* buf_allocated_size);

/**
 * Registers a callback function.
 *
 * @param fd
 * File descriptor to listen for events on.  Must be one of:
 *
 * - STDIN_FILENO
 * - STDOUT_FILENO
 * - STDERR_FILENO
 *
 * @param function [description]
 * Function to be invoked when data arrives on fd.
 *
 * @param ctx      [description]
 * User-defined argument to function.
 */
void register_io_callback(int fd, callback_fn function, void* ctx);

/**
 * Fork, exec, and start pumping data.
 */
void pump_execvp(char** argv);

/**
 * Child PID, from child forked in pump_execvp.
 */
extern int child_pid;
