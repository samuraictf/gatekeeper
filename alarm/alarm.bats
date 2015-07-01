#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "works with zero" {
    run alarm 0 sh -c 'sleep 1; echo hi'
    [ "$output" = "hi" ]
}

@test "kills properly" {
    run alarm 1 sh -c 'sleep 1; echo hi'
}
