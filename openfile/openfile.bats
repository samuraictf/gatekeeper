#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "open file on static fd" {
    run ./openfile <(echo hello) 1234 sh -c 'cat <&1234'
    [ "$output" = "hello" ]
}
