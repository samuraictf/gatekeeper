#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "stdin works" {
    run ./proxy cat <(echo hello)
    [ "$output" = "hello" ]
}

@test "stdout works" {
    run proxy echo hello
    [ "$output" = "hello" ]
}

@test "stdin filtering works" {
    run ./proxy cat <(echo AAAAA)
    [ "$output" = "aaaaa" ]
}
@test "stdin filtering works" {
    run ./proxy echo BBBBB
    [ "$output" = "bbbbb" ]
}
@test "additional hooks are called" {
    run ./proxy echo CCCCC
    [ "$output" = "ccccc" ]
}
@test "stderr filtering works" {
    run ./proxy sh -c 'echo DDDDD >/dev/stderr'
    [ "$output" = "ddddd" ]
}
