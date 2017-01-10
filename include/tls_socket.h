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

#ifndef tls_socket_h_
#define tls_socket_h_

#include <mutex>
#include <map>
#include <string>

#ifndef _WIN32
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif

#include <tls.h>

#ifndef _WIN32
#pragma clang pop
#define SOCKET int
#endif

#include <bravo/norm_socket.h>

namespace bravo
{
void set_server_cert_file(const std::string &server_cert_file);
void set_server_key_file(const std::string &server_key_file);
void set_client_ca_file(const std::string &client_ca_file);
    
std::string server_cert_file();
std::string server_key_file();
std::string client_ca_file();

class tls_init_t
{
public:
    tls_init_t();
    virtual ~tls_init_t();
    
    tls_config  *get_client_config(const std::string& cert = "");
    tls_config  *get_server_config(const std::string& cert = "", const std::string& key = "");
    std::map<std::string, tls_config*> configs;
    std::mutex  mtx;
};

class tls_socket : public norm_socket
{
public:
    tls_socket();
    tls_socket(SOCKET s, const std::string & cert_file = "", const std::string &key_file = "");
    virtual         ~tls_socket ();
    
    virtual int     read_       (char *buf, int count);
    virtual int     write_      (const char *buf, int count);
    virtual int     init        ();
    virtual int     close       ();
    virtual int     safe_tls_handshake(bool input = true, int timeout = default_timeout_);
    
private:
    std::string cert_file_;
    std::string key_file_;
    tls_config* client_config;
    tls_config* server_config;
    struct tls* tls;
};
    
}

#endif // tls_socket_h_

