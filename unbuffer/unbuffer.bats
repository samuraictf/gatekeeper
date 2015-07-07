#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PATH="$BATS_TEST_DIRNAME/example:$PATH"
    export FILE="${BATS_TMPDIR}file"
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
    example > "$FILE"
    run xxd "$FILE"
    [[ "$output" =~ "5868 656c 6c6f 0a" ]]
}

@test "${BATS_TEST_DIRNAME##*/}" {
    unbuffer example > "$FILE"
    run xxd "$FILE"
    [[ "$output" =~ "6865 6c6c 6f0d 0a58" ]]
}
















