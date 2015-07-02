#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export SETPGID=$BATS_TEST_DIRNAME/../setpgid/setpgid
}

@test "$(basename $BATS_TEST_DIRNAME) disabled" {
    uname -s | grep Darwin && skip

    echo FLAG > flag
    run sh -c 'read line < flag; sleep 0.1; echo $line; exit 3'

    [ "$status" = 3 ]
}

@test "$(basename $BATS_TEST_DIRNAME)" {
    uname -s | grep Darwin && skip

    echo FLAG > flag
    run $SETPGID inotify flag sh -c 'read line < flag; sleep 0.1; echo $line; exit 3'

    [ "$status" != 3 ]
}
















