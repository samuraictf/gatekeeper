#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
}

@test "${BATS_TEST_DIRNAME##*/}" {
    pcap foo.pcap echo hello
    run tcpdump -Ar foo.pcap
    [[ "$output" =~ "hello" ]]
}
