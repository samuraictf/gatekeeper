#pragma once

/**
 * Disables execve via seccomp.
 *
 * All attempts to execve will return an error.
 */
void no_execve();
