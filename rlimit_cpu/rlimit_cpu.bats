#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "rlimit cpu time maximu" {
    run rlimit_cpu 1 sh -c 'while true; do true; done'
    [ "$status" = 152 ]
}
