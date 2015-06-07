

if [ ! -d breakpad ];
then
    svn co http://google-breakpad.googlecode.com/svn/trunk/ breakpad
    pushd breakpad
    ./configure
    make -j
    popd
fi