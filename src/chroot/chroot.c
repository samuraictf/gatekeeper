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
#include <sys/syscall.h>
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
int mount_defaults           = 0;

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

void chroot_add_bind(char* path, char* chrootpath, int flags)
{
    real_bind[n_binds] = realpath(path, NULL);
    fake_bind[n_binds] = chrootpath;
    flag_bind[n_binds] = flags;
    n_binds++;
}

int real_tmp = 0;

void chroot_real_tmp() {
    real_tmp = 1;
}

void chroot_add_bind_defaults()
{
    mount_defaults = 1;
    chroot_add_bind("/bin", "/bin", MS_RDONLY);
    chroot_add_bind("/dev", "/dev", MS_RDONLY);
    chroot_add_bind("/etc", "/etc", MS_RDONLY);
    chroot_add_bind("/lib", "/lib", MS_RDONLY);
    chroot_add_bind("/lib32", "/lib32", MS_RDONLY);
    chroot_add_bind("/lib64", "/lib64", MS_RDONLY);
    chroot_add_bind("/sbin", "/sbin", MS_RDONLY);
    chroot_add_bind("/usr", "/usr", MS_RDONLY);

    if(real_tmp) {
        chroot_add_bind("/tmp", "/tmp", MS_RDONLY|MS_NOEXEC);
    }
}

int init_exitstatus = 0;
void init_term(int __attribute__ ((unused)) sig)
{
    _exit(init_exitstatus);
}

void chroot_waitpid(pid_t rootpid)
{
    pid_t pid;
    int status;
    /* so that we exit with the right status */
    signal(SIGTERM, init_term);

    while ((pid = wait(&status)) > 0) {
        /*
         * This loop will only end when either there are no processes
         * left inside our pid namespace or we get a signal.
         */
        if (pid == rootpid)
            init_exitstatus = status;
    }
    if (!WIFEXITED(init_exitstatus))
        _exit(1);
    _exit(WEXITSTATUS(init_exitstatus));
}

int chroot_invoke(char* directory)
{
    char buf[512];
    int status;
    int gid = getgid();
    int uid = getuid();

    unshare(CLONE_NEWUSER|CLONE_NEWNS|CLONE_NEWNET|CLONE_FS|CLONE_FILES|CLONE_NEWIPC|CLONE_NEWUTS);
    int child_pid = syscall(SYS_clone, CLONE_NEWPID | SIGCHLD, NULL);

    if(child_pid < 0) {
        return 1;
    }
    else if(child_pid > 0) {
        chroot_waitpid(child_pid);
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

    // avoid propagating mounts to or from the real root
    if(mount(NULL, "/", NULL, MS_PRIVATE|MS_REC, NULL) < 0) {
        puts("wOa3ljKHRnCSJD+a");
    }

    //
    // Bind all of the bind points
    //
    chdir(directory);

    for(size_t i = 0; i < n_binds; i++) {
        snprintf(buf, sizeof buf, ".%s", fake_bind[i]);
        mkdir(buf, 0555);
        mount(real_bind[i], buf, "bind", MS_BIND|MS_REC|MS_NOSUID| flag_bind[i], 0);
    }

    // We need these for mounts which may happen below
    mkdir("./tmp", 0777);
    mkdir("./proc", 0555);

    //
    // Remount the root as read-only.
    //
    chdir("..");
    snprintf(buf, sizeof buf, "%s.ro", directory);
    mkdir(buf, 0555);
    mount(directory, buf, "bind", MS_BIND|MS_REC, 0);
    mount(directory, buf, "none", MS_REMOUNT|MS_BIND|MS_RDONLY|MS_NOSUID, 0);
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

    // We need to unmount /proc in order to drop the procfs from
    // the parent namespace, even if we do not actually re-mount it.
    umount("/proc");

    if(mount_defaults) {
        if(!real_tmp) {
            mount("none", "/tmp", "tmpfs", 0, "size=128M,mode=777");
        }
        mount("proc",
              "/proc",
              "proc",
              MS_NODEV|MS_NOEXEC|MS_NOSUID|MS_RDONLY,
              NULL);
    }

    //
    // Drop all capabilities in the namespace so that there's no escape.
    //
    setresuid(uid,uid,uid);
    setresgid(gid,gid,gid);
    capdrop();

    return status;
}

