#!/bin/sh

cd ../apps/baretta
autom4te -l autotest test_version.at -o test_version
cd ../..
autoreconf --install
rm -rf autom4te.cache  
cd scripts


