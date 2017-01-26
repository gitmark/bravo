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

if [ ! -d /Users/$user/bravo-auto-build ]; then
	mkdir /Users/$user/bravo-auto-build
fi

cd /Users/$user/bravo-auto-build


if [ ! -d bravo ]; then
	git clone https://github.com/gitmark/bravo.git
fi

cd bravo
git pull
cid1=""

if [ -f commit_id ]; then
	cid1="`cat commit_id`"
fi

cid2="`git log --format=\"%H\" -n 1`"

#echo "cid1: $cid1";
#echo "cid2: $cid2";

if [ "$cid1" == "$cid2" ]; then
	echo "Already built this commit: $cid2"
	exit 0
fi

echo "$cid2" > commit_id;

commit_log="`git log $cid1..HEAD`"

mkdir build
cd build

../configure
make
make check
cd tests
echo "Bravo test results are attached for commit: \n$cid2\n\n $commit_log\n\n" | mutt -a *.log -s "Bravo Test Results" -- $email

cd ../../..



