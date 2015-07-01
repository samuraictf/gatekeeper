#!/usr/bin/env bats

setup() {
    PORT=$((10000 + $RANDOM % 10000))
    export PORT
}

@test "works without tool" {
    nc -lp $PORT -e 'echo hi' &
    run nc localhost $PORT
    [ "$output" = "hi" ]
}

@test "blocks localhost" {
    nc -lp $PORT -e './blacklist echo hi' &
    run nc localhost $PORT
    [ "$output" = "No connections from localhost" ]
}
