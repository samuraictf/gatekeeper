#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "chroot.h"

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: %s <root> argv0 argv1\n", argv[0]);
        exit(1);
    }

    chroot_add_uid_mapping(getuid(), getuid());
    chroot_add_gid_mapping(getgid(), getgid());

    // // Map all supplementary groups
    // gid_t groups[128];
    // size_t n_gids = getgroups(128, groups);
    // for(size_t i = 0; i < n_gids; i++) {
    //     chroot_add_gid_mapping(groups[i], groups[i]);
    // }

    chroot_add_bind_defaults();
    chroot_add_bind("home", "/home", 0);
    chroot_invoke(argv[1]);

    execvp(argv[2], &argv[2]);
}
