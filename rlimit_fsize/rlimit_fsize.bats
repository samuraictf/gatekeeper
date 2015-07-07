#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "rlimit disk size ok" {
    run rlimit_fsize 30 dd if=/dev/zero of=file bs=1 count=4
    [ "$status" = 0 ]
}

@test "rlimit disk size max" {
    run rlimit_fsize 3 dd if=/dev/zero of=file bs=1 count=4
    [ "$status" = 153 ]
}
