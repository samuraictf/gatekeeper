#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "regex empty file" {
    echo > "$BATS_TMPDIR/pattern"
    run regex "$BATS_TMPDIR/pattern" echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}


@test "regex passthru" {
    echo 'he..o' > "$BATS_TMPDIR/pattern"
    run regex "$BATS_TMPDIR/pattern" echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}

@test "regex match" {
    echo 'he[l]+o' > "${BATS_TMPDIR}pattern"
    run regex "$BATS_TMPDIR/pattern" echo hello
    [[ "$output" =~ "Sorry, Dave" ]]
    [ "$status" = 255 ]
}
