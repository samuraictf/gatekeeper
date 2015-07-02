#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "devctf wrong fd" {
    uname -s | grep Darwin && skip
	run env LD_PRELOAD="$BATS_TEST_DIRNAME/devctf.so" bash -c 'exec 1022</dev/urandom; head -c1 /dev/ctf || exit 1'
	[ "$status" = 1 ]
}

@test "devctf success" {
    uname -s | grep Darwin && skip
	run env LD_PRELOAD="$BATS_TEST_DIRNAME/devctf.so" bash -c 'exec 1023</dev/urandom; head -c1 /dev/ctf || exit 1'
	[ "$status" = 0 ]
}
