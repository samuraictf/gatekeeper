#include <unistd.h>
#include <stdio.h>

int main() {
    void* pointer = &main;

    // Leak a pointer into any ELF module
    write(1, &pointer, sizeof(pointer));

    // Arbitrary read access
    while(1) {
        read(0, &pointer, sizeof(pointer));
        write(1, pointer, sizeof(pointer));
    }
}
