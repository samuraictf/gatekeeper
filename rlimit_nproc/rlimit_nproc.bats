#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "rlimit nproc ok" {
    run rlimit_nproc -1 sh -c '(true)'
    [ "$status" = 0 ]
}

@test "rlimit disk size max" {
    run rlimit_nproc 0 sh -c '(true)'
    [ "$status" = 128 ]
}
