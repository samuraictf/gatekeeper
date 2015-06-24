#include <unistd.h>

int main() {
    void * ptr = main;
    write(1, &ptr, sizeof(ptr));
    while(read(0, &ptr, sizeof(ptr)) == sizeof(ptr)) {
        write(1, ptr, sizeof(ptr));
    }
}
