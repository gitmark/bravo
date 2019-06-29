#!/bin/bash

if [ -e cert-results ]; then
	rm -rf cert-results
fi

mkdir cert-results
cp localhost.pem localhost.key rootCA.pem rootCA.key cert-results/
cd cert-results
mv localhost.pem demohost.pem
mv localhost.key demohost.key
mv rootCA.pem demorootca.pem
mv rootCA.key demorootca.key
cd ..

cp -a cert-results /mnt/hgfs/vmshare/


