#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "capdrop.h"

#define BIND_POINTS_MAX 64
size_t  bind_points_count = 0;
char * bind_points[BIND_POINTS_MAX];

#define UID_MAP_MAX 64
size_t n_uids = 0;
uid_t real_uids[UID_MAP_MAX] = {0};
uid_t fake_uids[UID_MAP_MAX] = {0};

#define GID_MAP_MAX 64
size_t n_gids = 0;
gid_t real_gids[UID_MAP_MAX] = {0};
gid_t fake_gids[UID_MAP_MAX] = {0};

int permit_forking_in_chroot = 1;

void chroot_add_uid_mapping(int real, int fake)
{
    real_uids[n_uids] = real;
    fake_uids[n_uids] = fake;
    n_uids++;
}

void chroot_add_gid_mapping(int real, int fake)
{
    real_gids[n_gids] = real;
    fake_gids[n_gids] = fake;
    n_gids++;
}


void chroot_block_forking()
{
    permit_forking_in_chroot = 0;
}

void chroot_add_bind(char* path)
{
    bind_points[bind_points_count] = path;
    bind_points_count++;
}

void chroot_add_bind_defaults()
{
    chroot_add_bind("/bin");
    chroot_add_bind("/dev");
    chroot_add_bind("/etc");
    chroot_add_bind("/lib");
    chroot_add_bind("/lib32");
    chroot_add_bind("/lib64");
    chroot_add_bind("/proc");
    chroot_add_bind("/sbin");
    chroot_add_bind("/tmp");
    chroot_add_bind("/usr");
}

int chroot_invoke(char* directory)
{
    char buf[32];
    struct stat st;
    int status;

    //
    // The root directory should not be writeable by the current user.
    //
    if(0 != stat(directory, &st)) {
        puts("6KOhE9G1Hqo9WStz");
        status = 1;
    }

    // We should not own the root directory.
    if(st.st_uid == getuid()) {
        puts("KzANG220VzKa+SsY");
        status = 1;
    }

    chmod(directory, st.st_mode & ~(S_IWGRP | S_IWUSR | S_IWOTH));

    //
    // Create a new namespace
    //
    if(0 != unshare(CLONE_NEWUSER|CLONE_NEWIPC|CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWNET|CLONE_FS|CLONE_FILES)) {
        puts("kRtjpYi0N2SKnDlV");
        status = 1;
    }

    //
    // Map all of the UIDs
    //
    // File format of uid_map and gid_map is:
    //
    //  ID-inside-ns   ID-outside-ns   length
    //
    int fd = open("/proc/self/uid_map", O_RDWR);
    for(size_t i = 0; i < n_uids; i++) {
        size_t n = snprintf(buf, sizeof buf, "%u %u 1\n", fake_uids[i], real_uids[i]);
        if(0 <= write(fd, buf, n)) {
            status = 1;
        }
    }
    close(fd);

    fd = open("/proc/self/setgroups", O_RDWR);
    write(fd, buf, snprintf(buf, sizeof buf, "deny\n"));
    close(fd);

    fd = open("/proc/self/gid_map", O_RDWR);
    for(size_t i = 0; i < n_gids; i++) {
        size_t n = snprintf(buf, sizeof buf, "%u %u 1\n", fake_gids[i], real_gids[i]);
        if(0 <= write(fd, buf, n)) {
            status = 1;
        }
    }
    close(fd);


    //
    // Bind all of the bind points
    //
    chdir(directory);

    for(size_t i = 0; i < bind_points_count; i++) {
        snprintf(buf, sizeof buf, ".%s", bind_points[i]);
        mkdir(buf, 0111);
        mount(bind_points[i], buf, 0, MS_BIND|MS_REC, 0);
    }


    //
    // Close all file descriptors except for stdio,
    // to prevent any escapes.
    //
    for(int fd = 3; fd < 32; fd++) {
        close(fd);
    }

    //
    // Go into the chroot
    //
    if(0 != chroot(".")) {
        puts("VQRuvDgYaRupKvVa");
    }
    chdir("/");

    //
    // Drop all capabilities in the namespace so that there's no escape.
    //
    capdrop();

    if(permit_forking_in_chroot && fork() > 0) {
        int status;
        wait(&status);
        exit(status);
    }

    return status;
}
