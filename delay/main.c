#include <stdio.h>
#include <unistd.h>
#include "proxy.h"
#include "delay.h"


int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <seconds> argv0 argv1\n", argv[0]);
        exit(1);
    }

    long milliseconds = (1000.0 * atof(argv[1]));
    delay_stdout(milliseconds);
    delay_stdin(milliseconds);

    pump_execvp(&argv[2]);
}
