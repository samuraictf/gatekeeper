#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "rlimit disk size ok" {
    run rlimit_fsize 30 sh -c 'echo aaaaa > file'
    [ "$status" = 0 ]
}

@test "rlimit disk size max" {
    run rlimit_fsize 3 sh -c 'echo aaaaa > file'
    [ "$status" = 153 ]
}
