#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
int main() {
    int pid = fork();
    if(pid != 0) {
        dprintf(1, "%i\n", pid);
    }
}
