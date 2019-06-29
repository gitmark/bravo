#!/bin/bash

openssl genrsa -out localhost.key 2048
openssl req -new -nodes -key localhost.key -out localhost.csr  -config openssl.cnf.new -subj "/C=US/ST=Maryland/CN=demohost/OU=Demo Host/O=Demo Host/L=Baltimore/ST=Maryland/C=US"
openssl ca -config openssl.cnf.new -out localhost.crt -days 365 -policy policy_anything -extensions v3_req -infiles localhost.csr









