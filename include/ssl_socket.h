/*******************************************************************************
 MIT License
 
 Copyright (c) 2017 Mark Elrod
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 *******************************************************************************/

#ifndef ssl_socket_h_
#define ssl_socket_h_

#include <string>

#ifndef _WIN32
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif

#include <openssl/ssl.h>

#ifndef _WIN32
#pragma clang pop
#define SOCKET int
#endif

#include <bravo/norm_socket.h>

namespace bravo
{
class ssl_init_t
{
public:
    ssl_init_t();
    virtual ~ssl_init_t();
    
    BIO *outbio;
    SSL_CTX *client_ctx;
    SSL_CTX *server_ctx;
};

class ssl_socket : public norm_socket
{
public:
    ssl_socket();
    ssl_socket(SOCKET s, const std::string & cert_file = "", const std::string &key_file = "");
    virtual         ~ssl_socket ();
    
    virtual int     read_       (char *buf, int count);
    virtual int     write_      (const char *buf, int count);
    virtual int     init        ();
    virtual int     close       ();
    virtual int     safe_ssl_handshake(bool input = true, int timeout = default_timeout_);
    
private:
    std::string cert_file_;
    std::string key_file_;
    SSL* ssl;
};

}

#endif // ssl_socket_h_

