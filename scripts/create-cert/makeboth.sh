#!/bin/bash

if [ ! -e /etc/pki/tls/openssl.cnf.123123 ]; then
sudo mv /etc/pki/tls/openssl.cnf /etc/pki/tls/openssl.cnf.123123
fi

rm -rf newcerts
rm -rf certs
rm -rf crl
rm index.txt
rm serial

touch index.txt
mkdir -p newcerts
mkdir -p certs
mkdir -p crl
echo 00 > serial


sudo cp openssl.cnf.new /etc/pki/tls/openssl.cnf

./makerootcert.sh
./makelocalcert.sh
./topem.sh
./cpfiles.sh


if [ -e /etc/pki/tls/openssl.cnf.123123 ]; then
sudo mv /etc/pki/tls/openssl.cnf.123123 /etc/pki/tls/openssl.cnf
fi

