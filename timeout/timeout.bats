#!/usr/bin/env bats

setup() {
    make
}

@test "works with zero" {
    run ./alarm 0 sh -c 'sleep 1; echo hi'
    [ "$output" = "hi" ]
}

@test "kills properly" {
    run ./alarm 1 sh -c 'sleep 1; echo hi'
}
