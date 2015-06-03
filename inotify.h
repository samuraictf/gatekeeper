#ifndef _INOTIFY_H
#define _INOTIFY_H

#include <sys/inotify.h>
#include <linux/limits.h>

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (100 * (EVENT_SIZE + PATH_MAX))

void start_inotify_handler(char * keyfile, sighandler_t handler);
#endif
