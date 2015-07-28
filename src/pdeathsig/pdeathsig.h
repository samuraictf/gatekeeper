#include <signal.h>

/**
 * Sets the signal which is delivered to this process when
 * its parent dies.  This settings is not inherited across
 * fork()/clone()/vfork().
 */
void set_pdeathsig(int signal);
