#!/bin/sh

user="$1"
email="$2"

if [ "$user" == "" ]; then
	exit 1
fi

if [ "$email" == "" ]; then
	exit 1
fi

export PATH="/usr/local/bin:$PATH"

cd /Users/$user

if [ -d /Users/$user/bravo-test-build ]; then
	rm -rf /Users/$user/bravo-test-build
fi

mkdir bravo-test-build
cd bravo-test-build

git clone https://github.com/gitmark/bravo.git

cd bravo

mkdir build
cd build

../configure
make
make check
cd tests
echo "Bravo test results are attached." | mutt -a *.log -s "Bravo Test Results" -- $email

cd ../../..

if [ -d /Users/$user/bravo-test-build ]; then
	rm -rf /Users/$user/bravo-test-build
fi


