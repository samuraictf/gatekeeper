#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "chroot starts at /" {
    run ./noparent sh -c 'echo $PPID'
    [ "$output" = "1" ]
}
