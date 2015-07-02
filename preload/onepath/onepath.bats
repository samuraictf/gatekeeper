#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "onepath enforces proc/self/exe" {
    uname -s | grep Darwin && skip
    run env \
    	LD_PRELOAD=$BATS_TEST_DIRNAME/onepath.so \
        ONE_TRUE_PATH=/bin/bash \
    	bash <<EOF
echo A
/bin/echo B
EOF

    [ "$output" = "A" ]
}

