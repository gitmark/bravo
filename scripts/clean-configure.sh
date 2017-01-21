#!/bin/sh
cd ..
rm -rf config.h.in COPYING INSTALL Makefile.in aclocal.m4 ar-lib autom4te.cache compile depcomp install-sh Makefile.in missing test-driver configure 

cd apps/baretta
rm -rf Makefile.in

cd ../serrano
rm -rf Makefile.in

cd ../../tests
rm -rf Makefile.in testsuite1 testsuite2

cd ../src
rm -rf Makefile.in

cd ../scripts


