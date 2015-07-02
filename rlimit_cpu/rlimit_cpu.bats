#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"

    export CODE=137
    if uname -s | grep Darwin; then
        export CODE=152
    fi
}

@test "rlimit cpu time maximu" {
    run rlimit_cpu 0 sh -c 'while true; do true; done'
    [ "$status" = $CODE ]
}
