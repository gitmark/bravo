#!/bin/bash

openssl x509 -in localhost.crt -out localhost.pem -outform PEM
openssl x509 -in rootCA.crt -out rootCA.pem -outform PEM

