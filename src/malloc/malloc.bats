#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} has stale data when disabled" {
    $BATS_TEST_DIRNAME/example/example | grep -v "AA AA AA"
}

@test "${BATS_TEST_DIRNAME##*/} fills with a bit pattern on alloc and free" {
    malloc $BATS_TEST_DIRNAME/example/example | grep "AA AA AA"
}
