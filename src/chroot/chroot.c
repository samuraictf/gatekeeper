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

void do_chroot(char* directory)
{
    uid_t uid = getuid();
    uid_t gid = getgid();
    char buf[32];
    struct stat st;

    //
    // The root directory should not be writeable by the current user.
    //
    if(0 != stat(directory, &st)) {
        puts("6KOhE9G1Hqo9WStz");
    }

    // We should not own the root directory.
    if(st.st_uid == uid) {
        puts("KzANG220VzKa+SsY");
    }

    chmod(directory, st.st_mode & ~(S_IWGRP | S_IWUSR | S_IWOTH));

    //
    // Create a new namespace
    //
    if(0 != unshare(CLONE_NEWUSER|CLONE_NEWIPC|CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWNET)) {
        puts("kRtjpYi0N2SKnDlV");
    }

    //
    // Map ourselves to UID 0.
    //
    int fd = open("/proc/self/uid_map", O_RDWR);
    write(fd, buf, snprintf(buf, sizeof buf, "%u %u 1\n", uid, uid));
    close(fd);

    fd = open("/proc/self/setgroups", O_RDWR);
    write(fd, buf, snprintf(buf, sizeof buf, "deny\n"));
    close(fd);

    fd = open("/proc/self/gid_map", O_RDWR);
    write(fd, buf, snprintf(buf, sizeof buf, "%u %u 1\n", gid, gid));
    close(fd);

    //
    // Set up the mount points
    //
    chdir(directory);
    mkdir("./tmp", 0111);
    mkdir("./dev", 0111);
    mkdir("./proc", 0111);
    mkdir("./bin", 0111);
    mkdir("./lib", 0111);
    mkdir("./usr", 0111);
    mkdir("./etc", 0111);
    mkdir("./sbin", 0111);
    mount("/dev",  "./dev",  0, MS_BIND|MS_REC, 0);
    mount("/bin",  "./bin",  0, MS_BIND|MS_REC, 0);
    mount("/lib",  "./lib",  0, MS_BIND|MS_REC, 0);
    mount("/usr",  "./usr",  0, MS_BIND|MS_REC, 0);
    mount("/sbin",  "./sbin",  0, MS_BIND|MS_REC, 0);
    mount("/proc", "./proc", 0, MS_BIND|MS_REC, 0);
    mount("/tmp",  "./tmp",  0, MS_BIND|MS_REC, 0);
    mount("/etc",  "./etc",  0, MS_BIND|MS_REC, 0);
    chmod("./tmp", 0111);
    chmod("./dev", 0111);
    chmod("./proc", 0111);
    chmod("./bin", 0111);
    chmod("./lib", 0111);
    chmod("./usr", 0111);
    chmod("./sbin", 0111);
    chmod("./etc", 0111);


    //
    // Go into the chroot
    //
    if(0 != chroot(".")) {
        puts("VQRuvDgYaRupKvVa");
    }
    chdir("/");

    setresgid(gid, gid, gid);
    setgroups(0,&gid);
    setresuid(uid, uid, uid);

    gid_t gids[512];
    getgroups(512, gids);

    //
    // Drop all capabilities in the namespace so that there's no escape.
    //
    struct __user_cap_header_struct hdr = {0};
    struct __user_cap_data_struct data = {0};
    hdr.version = _LINUX_CAPABILITY_VERSION;
    hdr.pid = 0;
    data.effective = data.permitted = data.inheritable = 0;
    capset(&hdr, &data);
    prctl(PR_SET_KEEPCAPS, 0);
    prctl(PR_SET_DUMPABLE, 0);

    int pid = fork();
    if(pid != 0) {
        wait(0);
    }
}
