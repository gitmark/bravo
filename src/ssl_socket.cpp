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

// Good references ...
// https://wiki.openssl.org/index.php/Simple_TLS_Server
// http://codereview.stackexchange.com/questions/108600/complete-async-openssl-example
// http://stackoverflow.com/questions/3952104/how-to-handle-openssl-ssl-error-want-read-want-write-on-non-blocking-sockets
// http://stackoverflow.com/questions/31004997/how-to-make-non-blocking-openssl-connection
// https://wiki.openssl.org/index.php/SSL/TLS_Client
// https://github.com/daniloegea/libressl-ssl-api-examples/blob/master/server.c
// https://luxsci.com/blog/ssl-versus-tls-whats-the-difference.html
// https://stevehuston.wordpress.com/2009/12/29/how-to-use-schannel-for-ssl-sockets-on-windows/
// https://msdn.microsoft.com/en-us/library/aa375403(VS.85).aspx?tduid=(c91bdf34eab876444cf6d6aa67e1095f)(256380)(2459594)(TnL5HPStwNw-GhpegY.NZK6jruz.79tZuw)()
// http://searchsecurity.techtarget.com/definition/Microsoft-Schannel-Microsoft-Secure-Channel
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa374782(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa380516(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa380123(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa378812(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa375447(v=vs.85).aspx
// https://www.ibm.com/support/knowledgecenter/SSB23S_1.1.0.13/gtps7/s5sple2.html

#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0700f
    #endif
    typedef __int64 ssize_t;
    #define NOMINMAX
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/ioctl.h>
    #define INVALID_SOCKET -1
    #define WSAGetLastError() errno
    #define SOCKET_ERROR -1
    #define ioctlsocket ioctl
    #define WSAEWOULDBLOCK EWOULDBLOCK
    #include <netinet/tcp.h>
    #include <poll.h>
#endif

#include <sys/ioctl.h>
#include <mutex>
#include <thread>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <bravo/ssl_socket.h>
#include <bravo/tls_socket.h>
#include <bravo/string_utils.h>

using namespace std;

#define BACKLOG 20

namespace bravo
{
bool        is_ip           (const std::string &str);
SSL_CTX*    ssl_client_ctx  ();
SSL_CTX*    ssl_server_ctx  ();
int         thread_init     (void);
int         thread_cleanup  (void);

std::recursive_mutex ssl_init_mtx;
std::vector<std::recursive_mutex> ssl_mtx(CRYPTO_num_locks());

#define CH_UNK 0
#define CH_NUM 1
#define CH_DOT 2

int num_map[] =
{
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,2,0,
    1,1,1,1,    1,1,1,1,    1,1,0,0,    0,0,0,0,
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0
};

#define S_UNK   0
#define S_START (S_UNK + 1)
#define S_N1 (S_START + 1)
#define S_D1 (S_N1 + 1)
#define S_N2 (S_D1 + 1)
#define S_D2 (S_N2 + 1)
#define S_N3 (S_D2 + 1)
#define S_D3 (S_N3 + 1)
#define S_N4 (S_D3 + 1)

int state[][3] =
{
//   UNK   NUM     DOT
    { 0,    0,      0 }, // S_UNK
    { 0,    0,   S_N1 }, // S_START
    { 0, S_N1,   S_N2 }, // S_N1
    { 0, S_N2,   S_N3 }, // S_N2
    { 0, S_N3,   S_N4 }, // S_N3
    { 0, S_N4,      0 }, // S_N4
};

bool is_ip(const std::string &str)
{
    std::string s = bravo::trim(str);
    int st = S_START;
    int next = S_UNK;
    int n = 0;
    
    for(char c : str)
    {
        n = num_map[c];
        next = state[st][n];
        
        if (!next)
            break;

        st = next;
    }
    
    return st == S_N4;
}

int thread_init();
int thread_cleanup();

ssl_init_t::ssl_init_t()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    const SSL_METHOD *method = SSLv23_client_method();
    client_ctx = SSL_CTX_new(method);
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(client_ctx, flags);
    int r = SSL_CTX_load_verify_locations(client_ctx, client_ca_file().c_str(), 0);
    server_ctx = SSL_CTX_new( SSLv23_server_method());
    SSL_CTX_set_ecdh_auto(server_ctx, 1);
    r = SSL_CTX_use_certificate_file(server_ctx, server_cert_file().c_str(), SSL_FILETYPE_PEM);
    r = SSL_CTX_use_PrivateKey_file(server_ctx, server_key_file().c_str(), SSL_FILETYPE_PEM);
    thread_init();
}

ssl_init_t::~ssl_init_t()
{
    SSL_CTX_free(client_ctx);
    SSL_CTX_free(server_ctx);
    ERR_free_strings();
    EVP_cleanup();
    thread_cleanup();
}

ssl_init_t* get_ssl_init()
{
    static ssl_init_t ssl_init;
    return &ssl_init;
}

SSL_CTX* ssl_client_ctx()
{
    return get_ssl_init()->client_ctx;
}

SSL_CTX* ssl_server_ctx()
{
    return get_ssl_init()->server_ctx;
}

BIO* ssl_outbio()
{
    return get_ssl_init()->outbio;
}

//////////////////// ssl_socket //////////////////////////////////////////

ssl_socket::ssl_socket()
{
    ssl = nullptr;
};

ssl_socket::ssl_socket(SOCKET sock_, const std::string &cert_file, const std::string &key_file) :
cert_file_(cert_file),
key_file_(key_file),
norm_socket(sock_)
{
    ssl = nullptr;
}

ssl_socket::~ssl_socket()
{
    close();
}

int ssl_socket::read_(char *buf, int count)
{
    int actual = SSL_read(ssl, buf, count);

    if (actual < 0)
    {
        if (SSL_ERROR_WANT_READ == SSL_get_error(ssl, actual))
            return SOCKET_WANT_POLLIN;
    }

    return actual;
}

int ssl_socket::write_(const char *buf, int count)
{
    int actual = SSL_write(ssl, buf, count);

    if (actual < 0)
    {
        if (SSL_ERROR_WANT_WRITE == SSL_get_error(ssl, actual))
            return SOCKET_WANT_POLLOUT;
    }

    return actual;
}

int ssl_socket::close()
{
    if (ssl)
    {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = 0;
    }
    
    return norm_socket::close();
}

int ssl_socket::init()
{
    if(sock < 0)
    {
        sock = INVALID_SOCKET;
        return -1;
    }

    std::lock_guard<std::recursive_mutex> lock(ssl_init_mtx);
    
    if(mode_ == SM_SERVER)
    {
        SSL_CTX* s = ssl_server_ctx();
        ssl = SSL_new(s);
        SSL_set_fd(ssl, (int)sock);
        int ssl_err = SSL_accept(ssl);
        
        if (ssl_err <= 0)
        {
            ERR_print_errors_fp(stderr);
            close();
            return -1;
        }
        
        if(safe_ssl_handshake() != 0)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = 0;
            return -1;
        }

        int on = 1;
        int rc = ioctlsocket(sock, FIONBIO, (u_long*)&on);
        if (SOCKET_ERROR == rc)
            return -1;
    }
    else
    {
        SSL_CTX* c = ssl_client_ctx();
        ssl = SSL_new(c);
        SSL_set_fd(ssl, (int)sock);
        SSL_connect(ssl);
        const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4";
        SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
        
        if(safe_ssl_handshake() != 0)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = 0;
            return -1;
        }
    }
    
    initialized_ = true;
    return 0;
}

int ssl_socket::safe_ssl_handshake(bool input, int timeout)
{
    int c   = 0;
    int rc  = 0;
    
    while(c < timeout || timeout < 0)
    {
        rc = (int)SSL_do_handshake(ssl);
        
        if (rc != 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            
            if (timeout > 0 && c < timeout)
                ++c;
        }
        else
            break;
    }
    
    return 0;
}

}


