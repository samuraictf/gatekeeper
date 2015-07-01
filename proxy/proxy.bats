#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "stdin works" {
    run echo hello | proxy cat
    [ "$output" = "hello" ]
}

@test "stdout works" {
    run proxy echo hello
    [ "$output" = "hello" ]
}
