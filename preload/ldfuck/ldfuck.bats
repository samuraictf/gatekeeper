#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "ldfuck succeeds when not active" {
    uname -s | grep Darwin && skip
    run python \
    	$BATS_TEST_DIRNAME/example/demo.py \
    	$BATS_TEST_DIRNAME/example/example
	[ "$status" = 0 ]
}
@test "ldfuck causes failure when activated" {
    uname -s | grep Darwin && skip
    run env \
    	LD_PRELOAD=$BATS_TEST_DIRNAME/ldfuck.so \
    	python \
    	$BATS_TEST_DIRNAME/example/demo.py \
    	$BATS_TEST_DIRNAME/example/example
	[ "$status" = 1 ]
}

