# child control

Uses `seccomp` to make it easy to ensure that all children spawned can be easily killed, and that they cannot kill us.

## intended usage

The intended usage of this module is with something like the `timeout` module.  By invoking `install_child_control` after `fork`, we can kill all processes in the process group after a timeout, and no child processes can kill our monitor processes.
