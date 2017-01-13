#!/bin/sh

chmod 0755 *.sh

cd ../apps/baretta
chmod 0644 *.cpp

cd ../serrano
chmod 0644 *.cpp

cd ../../src
chmod 0644 *.cpp

cd ../include/bravo
chmod 0644 *.h

cd ../../docs
chmod 0644 *.txt

cd ..

chmod 0644 README.md LICENSE .gitignore


