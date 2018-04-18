#!/bin/bash

openssl genrsa -out localhost.key 2048
openssl req -new -nodes -key localhost.key -out localhost.csr  -config openssl.cnf.new
openssl ca -config openssl.cnf.new -out localhost.crt -days 500 -policy policy_anything -extensions v3_req -infiles localhost.csr







