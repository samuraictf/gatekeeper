#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PORT=$((10000 + $RANDOM % 10000))
}

@test "blocks localhost" {
    nc -lp $PORT -e 'blacklist echo hi' &
    run nc localhost $PORT
    [ "$output" = "No connections from localhost" ]
}
