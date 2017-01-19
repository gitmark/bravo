#!/bin/sh

chmod 0755 *.sh . ..
 
cd ../apps/baretta
chmod 0644 *
chmod 0755 . .. tests

cd ../serrano
chmod 0644 *
chmod 0755 . .. tests

cd ../../src
chmod 0644 *
chmod 0755 . ..

cd ../include/bravo
chmod 0644 *
chmod 0755 . ..

cd ../../docs
chmod 0644 *
chmod 0755 . ..

cd ..

chmod 0644 README.md LICENSE .gitignore ar-lib compile configure configure.ac depcomp install-sh Makefile.am missing package.m4 test-driver
chmod 0755 . .. tests configure

