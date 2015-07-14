#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PATH="$BATS_TEST_DIRNAME/example:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
	run example
	[ "$output" = "" ]
}

@test "${BATS_TEST_DIRNAME##*/}" {
    run env LD_PRELOAD="$BATS_TEST_DIRNAME/unsocket" "example"
    [ "$output" = "hello" ]
}
