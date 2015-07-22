#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} starts at /" {
    uname -s | grep Darwin && skip
    run chroot $BATS_TEST_DIRNAME/example sh -c 'echo "XX$(pwd)XX"'
    [[ "$output" =~ "/" ]]
}

@test "${BATS_TEST_DIRNAME##*/} cannot setuid root" {
    uname -s | grep Darwin && skip
    run chroot $BATS_TEST_DIRNAME/example sudo id
    [[ "$output" =~ "setuid" ]]
}

@test "${BATS_TEST_DIRNAME##*/} cannot setuid root" {
    uname -s | grep Darwin && skip
    run chroot $BATS_TEST_DIRNAME/example sudo id
    [[ "$output" =~ "setuid" ]]
}
