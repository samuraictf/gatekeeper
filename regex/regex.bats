#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "regex empty file" {
    echo > pattern
    run regex pattern echo hola
    [ "$output" = "hola" ]
}


@test "regex passthru" {
    echo he..o > pattern
    run regex pattern echo hola
    [ "$output" = "hola" ]
}

@test "regex match" {
    echo he..o > pattern
    run echo he..o > pattern && regex pattern echo hello
    echo "$output" > output
    [ "$output" = "Sorry, Dave.  I can't allow you to do that." ]
}
