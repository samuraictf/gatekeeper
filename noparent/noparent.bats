#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
    output=$(sh -c 'echo $PPID')
    [ "$output" = "$$" ]
}

@test "${BATS_TEST_DIRNAME##*/}" {
    output=$(noparent sh -c 'echo $PPID')
    [ "$output" != "$$" ]
}
