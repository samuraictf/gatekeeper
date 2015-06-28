#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "blacklist.h"

#ifdef __APPLE__
#define s6_addr32 __u6_addr.__u6_addr32
#endif

struct range {
    int af;
    char* start_str;
    char* stop_str;
    union {
        uint32_t start32[4];
        uint8_t start8[16];
    };
    union {
        uint32_t stop32[4];
        uint8_t stop8[16];
    };
};

#define MAX_RANGES 128
struct range* blacklisted_ranges[MAX_RANGES];
size_t blacklisted_range_count = 0;

struct range* parse_range(char* a, char* b) {
    int address_family = AF_INET;

    if(strchr(a, ':')) {
        address_family = AF_INET6;
    }

    struct range* r = calloc(1, sizeof(struct range));

    r->start_str = strdup(a);
    r->stop_str = strdup(b);
    r->af = address_family;
    inet_pton(address_family, a, &r->start8);
    inet_pton(address_family, b, &r->stop8);

    for(int i = 0; i < 4; i++) {
        r->start32[i] = ntohl(r->start32[i]);
        r->stop32[i] = ntohl(r->stop32[i]);
    }

    return r;
}

int check_range(struct range* r, struct sockaddr* address){
    if(!r || !address)
        return 0;

    if(address->sa_family != r->af)
        return 0;

    if(address->sa_family == AF_LOCAL ) {
        return 0;
    }

    // IPv4 is an easy comparison since the whole address fits into
    // a single integer.
    if(address->sa_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in*) address;

        if(r->start32[0] <= ntohl(sa->sin_addr.s_addr)
        && r->stop32[0] >= ntohl(sa->sin_addr.s_addr)) {
            return 1;
        }
    }

    // IPv6 is only slightly less easy.  As long as the DWORD is within
    // the bounds, it's guaranteed to be a match.  If it's on the boundary
    // of a match, we have to drill deeper.
    else if(address->sa_family == AF_INET6) {
        struct sockaddr_in6 *sa = (struct sockaddr_in6*) address;
        for(int i = 0; i < 4; i++) {
            uint32_t section = ntohl(sa->sin6_addr.s6_addr32[i]);
            uint32_t start = r->start32[i];
            uint32_t stop  = r->stop32[i];

            if(start <= section && section <= stop) {
                return 1;
            }

            if(start != section && stop != section) {
                break;
            }
        }
    }

    return 0;
}

void blacklist_range(char* start, char* stop)
{
    if(blacklisted_range_count >= MAX_RANGES) {
        puts("nXzi6LHYZfvGpHCl");
        return;
    }

    struct range* r = parse_range(start, stop);

    if(r == 0) {
        puts("6fQY3ktyzqAdPPz5");
        return;
    }

    blacklisted_ranges[blacklisted_range_count++] = r;
}

int blacklist_check_stdio() {
    for(int fd = STDOUT_FILENO; fd <= STDERR_FILENO; fd++) {
        socklen_t len = sizeof(struct sockaddr);
        struct sockaddr addr;

        memset(&addr, 0, sizeof(addr));

        if(0 != getpeername(fd, &addr, &len)) {
            if(errno != ENOTSOCK) {
                puts("AzhlScV2puvUQEnT");
            }
            continue;
        }

        for(size_t bl = 0; bl < blacklisted_range_count; bl++) {
            if(check_range(blacklisted_ranges[bl], &addr)) {
                return 1;
            }
        }
    }

    return 0;
}

void blacklist_parse(char* string)
{
    if(!string || !strlen(string))
        return;

    string = strdup(string);

    char * start = strtok(string, "-");
    char * stop  = strtok(NULL, "\n");

    while(start && stop) {
        blacklist_range(start, stop);

        start = strtok(NULL, "-");
        stop  = strtok(NULL, "\n");
    }

    free(string);
}
