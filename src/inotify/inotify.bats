#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export FLAG="${BATS_TMPDIR}flag"
    export SETPGID="${BATS_TEST_DIRNAME}../setpgid/setpgid"
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
    uname -s | grep Darwin && skip

    echo FLAG > "$FLAG"
    run sh -c 'read line < "$FLAG"; sleep 0.1; echo $line; exit 3'

    [ "$status" = 3 ]
}

@test "${BATS_TEST_DIRNAME##*/}" {
    uname -s | grep Darwin && skip

    echo FLAG > "$FLAG"
    run "$SETPGID" inotify "$FLAG" sh -c 'read line < "$FLAG"; sleep 0.1; echo $line; exit 3'

    [ "$status" != 3 ]
}
















