#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "noparent - disabled" {
    output=$(sh -c 'echo $PPID')
    [ "$output" = "$$" ]
}

@test "noparent" {
    output=$(noparent sh -c 'echo $PPID')
    [ "$output" != "$$" ]
}
