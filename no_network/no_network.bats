#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} works" {
    uname -s | grep Darwin && skip
    run no_network ping -c 1 localhost
    [ "$status" -eq 159 ]
}
