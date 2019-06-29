#!/bin/bash

openssl genrsa -out rootCA.key 4096
openssl req -x509 -new -nodes -key rootCA.key -sha256 -days 365 -subj "/CN=demorootca/OU=Demo Root CA/O=Demo Root CA/L=Baltimore/ST=Maryland/C=US" -out rootCA.crt

#-subj "/CN=demohost/OU=Demo Host/O=Demo Host/L=Baltimore/ST=Maryland/C=US"
#-subj "/C=GB/ST=London/L=London/O=Global Security/OU=IT Department/CN=example.com"


