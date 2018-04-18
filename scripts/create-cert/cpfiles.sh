#!/bin/bash

if [ -e cert-results ]; then
	rm -rf cert-results
fi

mkdir cert-results
cp localhost.pem localhost.key rootCA.pem rootCA.key cert-results
cp -a cert-results /mnt/hgfs/Desktop/


