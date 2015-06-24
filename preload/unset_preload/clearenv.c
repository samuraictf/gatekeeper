#include <stdlib.h>
#include <string.h>

__attribute__((constructor))
void initialize(int argc, char**argv, char**envp) {
    char* preload = getenv("LD_PRELOAD");
    if(preload) {
        memset(preload, 0, strlen(preload));
    }
}
