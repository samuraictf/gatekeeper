#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PATH="$BATS_TEST_DIRNAME/example:$PATH"
}

@test "$(basename $BATS_TEST_DIRNAME) disabled" {
    # The output is being captured, so stdout is a pipe..
    example > file
    run xxd file
    [[ "$output" =~ "5868 656c 6c6f 0a" ]]
}

@test "$(basename $BATS_TEST_DIRNAME)" {
    unbuffer example > file
    run xxd file
    [[ "$output" =~ "6865 6c6c 6f0d 0a58" ]]
}
















