#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
    run python $BATS_TEST_DIRNAME/example/demo.py $BATS_TEST_DIRNAME/example/example

    [ "$status" = 0 ]
}

@test "${BATS_TEST_DIRNAME##*/} enabled" {
    run got_nobind python $BATS_TEST_DIRNAME/example/demo.py $BATS_TEST_DIRNAME/example/example

    [ "$status" = 2 ]
}
