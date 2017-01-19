#!/bin/sh


#autom4te -l autotest ../apps/baretta/tests/test_version.at -o ../apps/baretta/tests/test_version
#autom4te -l autotest ../apps/baretta/tests/testsuite.at -o apps/baretta/tests/testsuite


#autom4te -l autotest ../apps/baretta/tests/test_version.at -o ../apps/baretta/tests/test_version
#autom4te -l autotest apps/serrano/tests/testsuite.at -o apps/serrano/tests/testsuite

cd ../tests

autom4te -l autotest testsuite.at -o testsuite

cd ..
autoreconf --install
rm -rf autom4te.cache  
cd scripts


