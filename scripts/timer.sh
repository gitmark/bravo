#!/bin/sh

while true
do
while [ $(date +%H:%M) != "$1" ]; do
date +%H:%M
    sleep 1
done
echo "Testing build"
make check
echo "Bravo test results are attached." | mutt -a *.log -s "Bravo Test Results" -- mark1@helixport.com

sleep 61
done