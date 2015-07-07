#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PATTERN="${BATS_TMPDIR}pattern"
}

@test "${BATS_TEST_DIRNAME##*/} empty file" {
    echo > "$PATTERN"
    run regex "$PATTERN" echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}


@test "${BATS_TEST_DIRNAME##*/} passthru" {
    echo 'he..o' > "$PATTERN"
    run regex "$PATTERN" echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}

@test "${BATS_TEST_DIRNAME##*/} match" {
    echo 'he[l]+o' > "$PATTERN"
    run regex "$PATTERN" echo hello
    [[ "$output" =~ "Sorry, Dave" ]]
    [ "$status" = 255 ]
}
