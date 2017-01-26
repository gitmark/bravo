#!/bin/sh

chmod 0644 *
chmod 0755 *.sh . ..
 
cd ../apps/baretta
chmod 0644 *
chmod 0755 . .. tests
cd tests
chmod 0644 *.at *.m4

cd ../../serrano
chmod 0644 *
chmod 0755 . .. tests
cd tests
chmod 0644 *.at *.m4

cd ../../../cmake/Modules
chmod 0644 *
chmod 0755 . ..

cd ../../src
chmod 0644 *
chmod 0755 . ..

cd ../include/bravo
chmod 0644 *
chmod 0755 . ..

cd ../../docs
chmod 0644 *
chmod 0755 . ..

cd ../tests
chmod 0644 *
chmod 0755 . .. *.sh testsuite1 testsuite2

cd ../m4
chmod 0644 .gitignore
chmod 0755 . ..

cd ..

chmod 0644 *
chmod 0755 . .. apps cmake docs include m4 scripts src tests 
chmod 0755 ar-lib compile configure depcomp install-sh missing test-driver

cd scripts

