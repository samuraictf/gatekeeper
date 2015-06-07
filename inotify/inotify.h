#ifndef _INOTIFY_H
#define _INOTIFY_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/inotify.h>
#include <linux/limits.h>
#include "ctf.h"

int log_fd;

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (100 * (EVENT_SIZE + PATH_MAX))
#define Log(...) ctf_writef(log_fd, __VA_ARGS__)

void start_inotify_handler(char * keyfile, __sighandler_t handler);
void signal_handler(int signo);
#endif
