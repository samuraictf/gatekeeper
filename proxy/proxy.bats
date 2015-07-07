#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "${BATS_TEST_DIRNAME##*/} stdin works" {
    run proxy cat <(echo hello)
    [ "$output" = "hello" ]
}

@test "${BATS_TEST_DIRNAME##*/} stdout works" {
    run proxy echo hello
    [ "$output" = "hello" ]
}

@test "${BATS_TEST_DIRNAME##*/} stdin filtering works" {
    run proxy cat <(echo AAAAA)
    [ "$output" = "aaaaa" ]
}
@test "${BATS_TEST_DIRNAME##*/} stdin filtering works" {
    run proxy echo BBBBB
    [ "$output" = "bbbbb" ]
}
@test "${BATS_TEST_DIRNAME##*/} additional hooks are called" {
    run proxy echo CCCCC
    [ "$output" = "ccccc" ]
}
@test "${BATS_TEST_DIRNAME##*/} unfiltered stderr works" {
    run sh -c 'echo DDDDD >/dev/stderr'
    [ "$output" = "DDDDD" ]
}

@test ${BATS_TEST_DIRNAME##*/} "stderr filtering works" {
    run proxy sh -c 'echo DDDDD >/dev/stderr; sleep 0.1'
    [ "$output" = "ddddd" ]
}
