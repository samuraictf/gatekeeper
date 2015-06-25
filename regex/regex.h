
#include <pcre.h>

/**
 * Callback type used with filter_regex_stdio to control what action
 * is taken when a match is found.
 *
 * @param buffer
 * Pointer to the full buffer which contains the match
 *
 * @param buffer_size
 * Size of the buffer
 *
 * @param match
 * Pointer to the first byte of the match within buffer.
 */
typedef void (*pcre_cb)(char* buffer, size_t buffer_size, char* match);

/**
 * Performs PCRE filtering on the specified file descriptor,
 * loading regular expressions from the specified file.
 *
 * @param fd
 * File descriptor, must be one of:
 * - STDIN_FILENO
 * - STDOUT_FILENO
 * - STDERR_FILENO
 *
 * @param file
 * Path to the file with the expressions to load.
 *
 * @param cb
 * Callback to invoke on a match.  May be NULL.
 */
void
filter_regex_stdio(int fd, char* file, pcre_cb cb);
