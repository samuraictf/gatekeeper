#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "regex empty file" {
    echo > pattern
    run regex pattern echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}


@test "regex passthru" {
    echo 'he..o' > pattern
    run regex pattern echo hola
    [ "$output" = "hola" ]
    [ "$status" = 0 ]
}

@test "regex match" {
    echo 'he[l]{2}o' > pattern
    run regex pattern echo hello
    [ "$output" != 0 ]
    [ "$status" = 255 ]
}
