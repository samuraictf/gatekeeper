#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "chroot starts at /" {
    uname -s | grep Darwin && skip
    run chroot . pwd
    [ "$output" = "/" ]
}
