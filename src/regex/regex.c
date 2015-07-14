#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "regex.h"
#include "proxy.h"

#define BUFFER_SIZE 1024

// One for each of stdin, stdout, stderr
typedef struct _pcre_data {
    int fd;
    regex_callback cb;
    pcre* expr;
    pcre_extra* extra;
    size_t used;
    char buffer[BUFFER_SIZE];
} pcre_data;


int max(int a, int b) {
    return a>b?a:b;
}

pcre* pcre_compile_errmsg(char* expression) {
    const char *errmsg = NULL;
    int erroff = 0;
    pcre* rv = pcre_compile(expression, PCRE_DOTALL, &errmsg, &erroff, 0);

    if(!rv) {
        dprintf(2, "PCRE error: %s\n"
                   "%s\n"
                   "%*s^--here\n",
                   errmsg, expression, erroff-1, "");
    }

    return rv;
}

pcre* load_expressions(char* filename) {
    size_t size = 0;
    struct stat st = {0};
    int fd = open(filename, O_RDONLY);

    if(fd < 0) {
        perror(filename);
        exit(-1);
    }

    if(-1 == fstat(fd, &st)) {
        perror("fstat");
        exit(-1);
    }

    if(!st.st_size) {
        return NULL;
    }

    char *mem = mmap(0, st.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if(!mem) {
        perror("mmap");
        exit(-1);
    }

    mem[st.st_size] = '\0';

    // Read all of them in.
    char **expressions = 0;
    char *cur = mem;
    char *end = mem + st.st_size;
    size_t n_expr = 0;

    char *expr = strtok(mem, "\n");

    while(expr) {
        size_t len = strlen(expr);

        if(expr[0] != '#' && len) {
            expressions = realloc(expressions, (n_expr+1) * sizeof(*expressions));

            if(!expressions) {
                perror("realloc");
                exit(-1);
            }

            expressions[n_expr++] = strdup(expr);
        }

        expr = strtok(NULL, "\n");
    };

    // Close the file, no longer needed
    munmap(mem, st.st_size);
    close(fd);

    // Test each of them individually as we build the complete
    // expression.  We need enough space for at most the entire
    // contents of the file, plus a '|' to join statements, and
    // '()' to surround each statement, and a NULL terminator.
    size_t total_size = st.st_size + (3 * n_expr) + 1;
    char *full_expr = calloc(1, total_size);

    for(size_t i = 0; i < n_expr; i++) {
        pcre *p = pcre_compile_errmsg(expressions[i]);

        if(p) {
            if(i != 0) {
                strcat(full_expr, "|");
            }
            strcat(full_expr, "(");
            strcat(full_expr, expressions[i]);
            strcat(full_expr, ")");
            pcre_free(p);
        }

        free(expressions[i]);
    }
    free(expressions);

    if(!strlen(full_expr))
        return NULL;

#ifdef DEBUG
    puts(full_expr);
#endif

    return pcre_compile_errmsg(full_expr);
}

void
perform_pcre_filter(int fd, void* ctx, void** buf, size_t *used, size_t *allocated)
{
    pcre_data* data = (pcre_data*) ctx;
    char* match;

    // Shift the buffer over to make room for the new data
    size_t bytes = *used;
    size_t available = sizeof(data->buffer) - data->used;

    if(bytes > available) {
        size_t shift_idx = bytes - available;
        size_t shift_size = sizeof(data->buffer) - shift_idx;

        memcpy(data->buffer, &data->buffer[shift_idx], shift_size);
        data->used -= shift_idx;
    }

    // Copy in the new data
    memcpy(&data->buffer[data->used], *buf, bytes);
    data->used += bytes;

    // Check for a match
    int result = pcre_exec(data->expr, data->extra, data->buffer, data->used, 0, 0, NULL, 0);

    if(result < 0) {
        if(result == PCRE_ERROR_NOMATCH)
            return;

        puts("bnQCYrHTkZvKVZis");
        exit(1);
    }

    if(data->cb) {
        data->cb(data->buffer, data->used, &data->buffer[result]);
    }

    return;
}

void
regex_filter_stdio(int fd, char* file, regex_callback cb)
{
    const char* errmsg = 0;

    if(fd < 0 || fd > 2) {
        puts("StYyoTcXXs2AcL9V");
        exit(1);
    }

    pcre_data* data = malloc(sizeof(pcre_data));

    data->expr   = load_expressions(file);

    if(!data->expr) {
        return;
    }

    data->extra  = pcre_study(data->expr, PCRE_STUDY_JIT_COMPILE, &errmsg);
    data->fd     = fd;
    data->used   = 0;
    data->cb     = cb;

    proxy_register_callback(fd, perform_pcre_filter, data);
}
