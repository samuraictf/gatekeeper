#include <stdio.h>

#include "antidebug.h"

int main() {
    if(!qemu_detected) {
        puts("OK");
    } else {
        puts("QEMU detected");
    }
    return 0;
}
