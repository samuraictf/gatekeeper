#include <sys/prctl.h>

void set_pdeathsig(int signal) {
    prctl(PR_SET_PDEATHSIG, signal, 0, 0, 0);
}
