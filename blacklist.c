#include "gatekeeper.h"

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

void parse_blacklist_file(char * filename) {
    FILE *f;
    char line[1024];
    int linenum;
    
    if ((f = fopen(filename, "r")) == NULL) {
        Log("Error opening %s for reading: %s\n", filename, strerror(errno));
        return;
    }

    linenum = 1;
    while (fgets(line, sizeof(line), f) != NULL && linenum < MAX_RANGES) {
        blacklist_parse(line);
        linenum++;
    }
    fclose(f);
    Log("Done.\nParsed %d blacklist ip inputs.\n", linenum);

}


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
        puts("max blacklist ranges exceeded");
        return;
    }

    struct range* r = parse_range(start, stop);

    if(r == 0) {
        puts("invalid blacklist range");
        return;
    }

    blacklisted_ranges[blacklisted_range_count++] = r;
}

int blacklist_check_stdio() {
    for(int fd = STDOUT_FILENO; fd <= STDERR_FILENO; fd++) {
        if (blacklist_check_fd(fd) == 1) {
            return 1;
        }
    }

    return 0;
}

int blacklist_check_fd(int fd) {
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr addr;

    memset(&addr, 0, sizeof(addr));

    if(0 != getpeername(fd, &addr, &len)) {
        if(errno != ENOTSOCK) {
            puts("AzhlScV2puvUQEnT");
        }
    }

    for(size_t bl = 0; bl < blacklisted_range_count; bl++) {
        if(check_range(blacklisted_ranges[bl], &addr)) {
            return 1;
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
