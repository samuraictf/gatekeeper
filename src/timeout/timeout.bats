#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} kills properly" {
    run timeout 1 sh -c 'sleep 1; echo hi'
}
