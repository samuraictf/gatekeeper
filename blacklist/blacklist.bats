#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export PORT=$((10000 + $RANDOM % 10000))
}

@test "blocks localhost (osx)" {
	uname -s | grep Linux && skip
    nc -lp $PORT -e './blacklist echo hi' &
    run nc localhost $PORT
    [ "$output" = "No connections from localhost" ]
}


@test "blocks localhost (linux)" {
	uname -s | grep Darwin && skip
    nc -lp $PORT -c './blacklist echo hi' &
    run nc localhost $PORT
    [ "$output" = "No connections from localhost" ]
}
