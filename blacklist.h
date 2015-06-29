#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

struct range;

void parse_blacklist_file(char * filename);
/**
 * Parses a string IP address range in the format of:
 *
 * 1.2.3.4-1.2.3.5 
 * or
 * 11.22.33.44-55.66.77.88
 *
 * @param  string
 * NULL-terminated string to parse.
 */
void blacklist_parse(char* string);

/**
 * Adds an IP range into the internal list of IPs to blacklist.
 */
void blacklist_range(char* start, char* stop);

/**
 * Checks whether any of the standard I/O sockets are blacklisted.
 *
 * @return 1 if any sockets are blacklisted, 0 otherwise
 */
int blacklist_check_stdio();

int blacklist_check_fd(int fd);
