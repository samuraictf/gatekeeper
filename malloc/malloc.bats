#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "malloc fills" {
    malloc $BATS_TEST_DIRNAME/example/example | grep "AA AA AA"
}
