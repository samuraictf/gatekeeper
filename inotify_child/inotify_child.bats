#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "$(basename $BATS_TEST_DIRNAME) disabled" {
    uname -s | grep Darwin && skip

    echo FLAG > flag
    run sh -c 'read line < flag; sleep 0.1; exit 3'

    [ "$status" = 3 ]
}

@test "$(basename $BATS_TEST_DIRNAME)" {
    uname -s | grep Darwin && skip

    echo FLAG > flag
    run inotify_child flag bash -c 'read line < flag; sleep 0.1; exit 3'

    [ "$status" != 3 ]
}
















