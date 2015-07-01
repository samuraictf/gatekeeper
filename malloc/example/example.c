#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hexdump(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}


int main() {
    void *memory = malloc(32);
    puts("Fresh allocation");
    hexdump(memory, 32);
    strcpy(memory, "Hello, world!");
    puts("After strcpy");
    hexdump(memory, 32);
    free(memory);
    puts("After free");
    hexdump(memory, 32);

    memory = malloc(32);
    puts("After 2nd malloc");
    hexdump(memory, 32);
    free(memory);
    puts("After 2nd free");
    hexdump(memory, 32);
    puts("Before double-free");
    free(memory);
    puts("After double-free");
}
