#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}


@test "${BATS_TEST_DIRNAME##*/} enforces proc/self/exe" {
    uname -s | grep Darwin && skip
    run env \
    	LD_PRELOAD=$BATS_TEST_DIRNAME/onepath \
        ONE_TRUE_PATH=/bin/bash \
    	bash <<EOF
builtin echo A
/bin/echo B
EOF

    [ "$output" = "A" ]
}

