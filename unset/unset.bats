#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/}" {
    uname -s | grep Darwin && skip
	run env LD_PRELOAD="${BATS_TEST_DIRNAME}/unset" sh -c 'echo "XX${LD_PRELOAD}XX"'
	[ "$output" = "XXXX" ]
}
