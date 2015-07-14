#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} ok" {
    run rlimit_nproc -1 "${BATS_TEST_DIRNAME}/example/example"
    [ "$output" != "-1" ]
}

@test "${BATS_TEST_DIRNAME##*/} max (linux)" {
    uname -s | grep Darwin && skip
    run rlimit_nproc 0 "${BATS_TEST_DIRNAME}/example/example"
    [ "$output" = "-1" ]
}


@test "${BATS_TEST_DIRNAME##*/} max (osx)" {
    uname -s | grep Linux && skip
    run rlimit_nproc 0 "${BATS_TEST_DIRNAME}/example/example"
    [ "$output" = "-1" ]
}
