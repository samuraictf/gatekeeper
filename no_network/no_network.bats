#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "deny network access" {
    uname -s | grep Darwin && skip
    run no_network ping -c 1 localhost
    [ "$status" -eq 159 ]
}
