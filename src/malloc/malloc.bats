#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} fills" {
    malloc $BATS_TEST_DIRNAME/example/example | grep "AA AA AA"
}
