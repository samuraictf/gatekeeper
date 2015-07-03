#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "got_bind disabled" {
    run python $BATS_TEST_DIRNAME/example/demo.py $BATS_TEST_DIRNAME/example/example

    [ "$status" = 0 ]
}

@test "got_bind enabled" {
    run got_bind python $BATS_TEST_DIRNAME/example/demo.py $BATS_TEST_DIRNAME/example/example

    [ "$status" = 2 ]
}
