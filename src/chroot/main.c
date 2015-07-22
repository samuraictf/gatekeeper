#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "chroot.h"

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <dir> argv0 argv1\n", argv[0]);
        exit(1);
    }

    chroot_add_uid_mapping(getuid(), getuid());
    chroot_add_gid_mapping(getgid(), getgid());
    chroot_add_bind_defaults();
    chroot_invoke(argv[1]);

    execvp(argv[2], &argv[2]);
}
