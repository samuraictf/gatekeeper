#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/}" {
    uname -s | grep Darwin && skip
	run env LD_PRELOAD="$BATS_TEST_DIRNAME/no_execve" sh -c 'echo hi; /bin/echo hello'
	[[ "$output" =~ "hi" ]]
	[[ ! "$output" =~ "hello" ]]
    [ "$status" = 2 ]
}
