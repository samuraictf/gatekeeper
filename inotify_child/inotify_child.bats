#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export FLAG="${BATS_TMPDIR}flag"
}

@test "$(basename \"$BATS_TEST_DIRNAME\") disabled" {
    uname -s | grep Darwin && skip

    echo FLAG > "$FLAG"
    run sh -c 'read line < "$FLAG"; sleep 0.1; exit 3'

    [ "$status" = 3 ]
}

@test "$(basename \"$BATS_TEST_DIRNAME\")" {
    uname -s | grep Darwin && skip

    echo FLAG > "$FLAG"
    run inotify_child "$FLAG" bash -c 'read line < "$FLAG"; sleep 0.1; exit 3'

    [ "$status" != 3 ]
}
















