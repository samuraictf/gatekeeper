#pragma once
#include <stdint.h>
#include <stdlib.h>

/**
 * I/O callback hook function type.
 *
 * @param fd
 * File descriptor which this callback pertains to.
 * For example, if it is set to STDIN_FILENO, buf points to new input.
 * If it is instead STDOUT_FILENO or STDERR_FILENO, buf points to
 * new output.
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
 * @notes
 * buf, buf_used, and buf_allocated_size may all be modified by
 * the callback.  buf should only be resized with realloc().
 */
typedef void (*proxy_callback_fn)(int fd,
                                   void* ctx,
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
void proxy_register_callback(int fd, proxy_callback_fn function, void* ctx);

/**
 * Fork and exec
 */
void proxy_fork_execvp(char** argv);

/**
 * Pump data
 */
void proxy_pump();

/**
 * Child PID, from child forked in pump_execvp.
 */
extern int proxy_child_pid;
