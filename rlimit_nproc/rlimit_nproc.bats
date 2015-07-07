#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} ok" {
    run rlimit_nproc -1 sh -c '(true)'
    [ "$status" = 0 ]
}

@test "${BATS_TEST_DIRNAME##*/} max (linux)" {
    uname -s | grep Darwin && skip
    run rlimit_nproc 0 sh -c '(true)'
    [ "$status" = 2 ]
}


@test "${BATS_TEST_DIRNAME##*/} max (osx)" {
    uname -s | grep Linux && skip
    run rlimit_nproc 0 sh -c '(true)'
    [ "$status" = 128 ]
}
