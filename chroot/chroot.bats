#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "chroot starts at /" {
    uname -s | grep Darwin && skip
    run chroot $BATS_TEST_DIRNAME/example sh -c 'echo $PWD'
    [ "$output" = "/" ]
}
