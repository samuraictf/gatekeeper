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
#include <linux/fs.h>

#include "capdrop.h"

#define BIND_POINTS_MAX 64
size_t  n_binds = 0;
char * real_bind[BIND_POINTS_MAX];
char * fake_bind[BIND_POINTS_MAX];
int    flag_bind[BIND_POINTS_MAX];

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

void chroot_add_bind(char* realpath, char* chrootpath, int flags)
{
    real_bind[n_binds] = realpath;
    fake_bind[n_binds] = chrootpath;
    flag_bind[n_binds] = flags;
    n_binds++;
}

void chroot_add_bind_defaults()
{
    chroot_add_bind("/bin", "/bin", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/dev", "/dev", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/etc", "/etc", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/lib", "/lib", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/lib32", "/lib32", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/lib64", "/lib64", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/proc", "/proc", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/sbin", "/sbin", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/tmp", "/tmp", MS_NOSUID|MS_RDONLY);
    chroot_add_bind("/usr", "/usr", MS_NOSUID|MS_RDONLY);
}

int chroot_invoke(char* directory)
{
    char buf[32];
    int status;

    //
    // Create a new namespace
    //
    if(0 != unshare(CLONE_NEWUSER
                  | CLONE_NEWIPC
                  | CLONE_NEWNS
                  | CLONE_NEWPID
                  | CLONE_NEWUTS
                  | CLONE_NEWNET
                  | CLONE_FS
                  | CLONE_FILES)) {
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

    for(size_t i = 0; i < n_binds; i++) {
        snprintf(buf, sizeof buf, ".%s", fake_bind[i]);
        mkdir(buf, 0555);
        mount(real_bind[i], buf, 0, MS_BIND|MS_REC | flag_bind[i], 0);
    }

    //
    // Remount the root as read-only.
    //
    chdir("..");
    snprintf(buf, sizeof buf, "%s.ro", directory);
    mkdir(buf, 0555);
    mount(directory, buf, "bind", MS_BIND|MS_REC, 0);
    mount(directory, buf, "none", MS_REMOUNT|MS_BIND|MS_RDONLY, 0);
    directory = buf;
    chdir(directory);

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

