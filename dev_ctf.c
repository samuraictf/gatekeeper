

/*
 * Make /dev/ctf available on a constant file descriptor,
 * such that it is available within a chroot.
 */
__attribute__((constructor))
void
open_dev_ctf()
{
    int fd = open("/dev/ctf", O_RDONLY);
    dup2(1023, fd);
    close(fd);
}