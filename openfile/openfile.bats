#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "open file on static fd" {
    run openfile <(echo hello) 666 bash -c 'cat <&666'
    [ "$output" = "hello" ]
}
