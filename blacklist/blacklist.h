#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

struct range;

/**
 * Parses a file of IP address ranges in the format of:
 *
 * 1.2.3.4-1.2.3.5<NEWLINE>
 * 11.22.33.44-55.66.77.88<NEWLINE>
 *
 * @param  path
 * Path to the file
 *
 * @return
 * A NULL-terminated list of range objects.
 */
struct range** parse_file(char* path);

/**
 * Parses a single address range.
 * @param  a First address in the range.
 * @param  b Last address in the range.
 * @return Pointer to a range object.
 */
struct range* parse_range(char* a, char* b);

/**
 * Checks a packed IP address for being in an IP range.
 * @param  range   Range object
 * @param  address Sockaddr object
 * @param  size    Size of the sockaddr object, as returned by e.g. getpeername.
 * @return         1 if the address is a member
 */
int check_range(struct range* r, struct sockaddr* address);

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
