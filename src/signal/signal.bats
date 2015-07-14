#!/usr/bin/env bats

setup() {
    export PATH="$BATS_TEST_DIRNAME:$PATH"
    export SETPGID=$BATS_TEST_DIRNAME/../setpgid/setpgid
}

@test "${BATS_TEST_DIRNAME##*/} disabled" {
    run bash -c 'echo Hello; kill -SIGTERM $$; echo Goodbye'
    [ "$status" = 143 ]
}

@test "${BATS_TEST_DIRNAME##*/}" {
    run signal bash -c 'echo Hello; kill -SIGTERM $$; echo Goodbye'
    [ "$status" = 0 ]
}
