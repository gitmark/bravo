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

#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0700f
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef __int64 ssize_t;
#else
    #include <poll.h>
    #define INVALID_SOCKET -1
#endif

#include <thread>
#include <openssl/err.h>
#include <bravo/tls_socket.h>

using namespace std;
using namespace bravo;

#define BACKLOG 20

bool is_ip(const std::string &str);
std::string server_cert_file_;
std::string server_key_file_;
std::string client_ca_file_;

std::recursive_mutex tls_init_mtx;
std::vector<std::recursive_mutex> tls_mtx(CRYPTO_num_locks());

void set_server_cert_file(const std::string &server_cert_file)
{
    server_cert_file_ = server_cert_file;
}

void set_server_key_file(const std::string &server_key_file)
{
    server_key_file_ = server_key_file;
}

void set_client_ca_file(const std::string &client_ca_file)
{
    client_ca_file_ = client_ca_file;
}

static void locking_function(int mode, int n, const char *file, int line)
{
    if(mode & CRYPTO_LOCK)
        tls_mtx[n].lock();
    else
        tls_mtx[n].unlock();
}

int thread_init(void)
{
    CRYPTO_set_locking_callback(locking_function);
    return 0;
}

int thread_cleanup(void)
{
    CRYPTO_set_locking_callback(NULL);
    return 0;
}

tls_config* tls_server_config_(std::string cert_file = "", std::string key_file = "")
{
    tls_config* server_config_ = tls_config_new();
    
    if(server_config_ == nullptr)
        return nullptr;
    
    unsigned int protocols = 0;
    
    if(tls_config_parse_protocols(&protocols, "secure") < 0)
        return nullptr;
    
    tls_config_set_protocols(server_config_, protocols);
    
    const char *ciphers = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384";
    
    if(tls_config_set_ciphers(server_config_, ciphers) < 0)
        return nullptr;
    
    if (!key_file.size())
        key_file = server_key_file_;
    
    if (!cert_file.size())
        cert_file = server_cert_file_;

    if(tls_config_set_cert_file(server_config_, cert_file.c_str()) < 0)
        return nullptr;

    if(tls_config_set_key_file(server_config_, key_file.c_str()) < 0)
        return nullptr;
    
    return server_config_;
}

tls_config *tls_client_config_(std::string cert = "")
{
    tls_config *client_config_ = tls_config_new();
    
    if(client_config_ == NULL)
        return nullptr;

    unsigned int protocols = 0;
    
    if(tls_config_parse_protocols(&protocols, "secure") < 0)
        return nullptr;

    tls_config_set_protocols(client_config_, protocols);
    
    const char *ciphers = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384";

    if(tls_config_set_ciphers(client_config_, ciphers) < 0)
        return nullptr;
    
    if (!cert.size())
        cert = client_ca_file_;
    
    if(tls_config_set_ca_file(client_config_, cert.c_str()) < 0)
        return nullptr;
    
    return client_config_;
}

tls_init_t::tls_init_t()
{
    // Good reference:
    // https://github.com/daniloegea/libressl-tls-api-examples/blob/master/server.c
    
    std::unique_lock<std::mutex> lock(mtx);

    if(tls_init() < 0)
        return;

    thread_init();
}

tls_init_t::~tls_init_t()
{
    std::unique_lock<std::mutex> lock(mtx);
    
    for(auto &p : configs)
        tls_config_free(p.second);

    thread_cleanup();
}

tls_config *tls_init_t::get_client_config(const std::string& cert)
{
    std::unique_lock<std::mutex> lock(mtx);
    
    if (configs.count(cert))
        return configs[cert];
    
    tls_config *config = tls_client_config_(cert);
    configs[cert] = config;
    return config;
}

tls_config *tls_init_t::get_server_config(const std::string& cert, const std::string& key)
{
    std::string k = cert + ":" + key;
    std::unique_lock<std::mutex> lock(mtx);
    
    if (configs.count(k))
        return configs[k];
    
    tls_config *config = tls_server_config_(cert, key);
    configs[k] = config;
    return config;
}

tls_init_t* get_tls_init()
{
    static tls_init_t tls_init;
    return &tls_init;
}

tls_config* tls_client_config(const std::string &cert = "")
{
    return get_tls_init()->get_client_config(cert);
}

tls_config* tls_server_config(const std::string &cert = "", const std::string &key = "")
{
    return get_tls_init()->get_server_config(cert, key);
}

int data_match(const char *data, int len, const char *end_marker, int end_len, int &mcount)
{
    // Returns the index of the beginning of the found string.
    // >= 0 means it was found and the return value is a valid index.
    // -1 means the end marker was not found.
    // mcount is used and updated. It indicatea how far into the match we are
    if (!data || !end_marker || len < 1 || end_len < 1)
        return -1;
    
    const char* d; // data
    const char* e; // end marker
    const char* start = data;
    const char* ee = end_marker + end_len;
    const char* de = data + len;
    d = start;
    e = end_marker + mcount; // Pick up where we left off last time, only for first iteration.

    while(*d == *e && e != ee && d != de)
    {
        ++d;
        ++e;
    }
    
    if(e == ee || d == de)
    {
        mcount = (int)(e - end_marker);
        return (int)(d - data);
    }
    
    ++start;
    
    while(start != de)
    {
        mcount = 0;
        d = start;
        e = end_marker;
        
        while(*d == *e && e != ee && d != de)
        {
            ++d;
            ++e;
        }
        
        if(e == ee || d == de)
        {
            mcount = (int)(e - end_marker);
            return (int)(d - data);
        }
        
        ++start;
    }
    
    mcount = 0;
    return -1 ;
}

//////////////////// tls_socket //////////////////////////////////////////

tls_socket::tls_socket()
{
    client_config = nullptr;
    server_config = nullptr;
    tls = nullptr;
};

tls_socket::tls_socket(SOCKET sock_, const std::string &cert_file, const std::string &key_file) :
cert_file_(cert_file),
key_file_(key_file),
norm_socket(sock_)
{
    client_config = nullptr;
    server_config = nullptr;
    tls = nullptr;
}

tls_socket::~tls_socket()
{
    close();
}

int tls_socket::read_(char *buf, int count)
{
    return (int)tls_read(tls, buf, count);
}

int tls_socket::write_(const char *buf, int count)
{
    return (int)tls_write(tls, buf, count);
}

int tls_socket::close()
{
    if(tls)
    {
        tls_close(tls);
        tls_free(tls);
        tls = 0;
    }

    if (server_config)
        server_config = nullptr;
    
    if (client_config)
        client_config = nullptr;
    
    return norm_socket::close();
}

int tls_socket::safe_tls_handshake(bool input, int timeout)
{
    bool *wants_poll;
    int poll_type;
    int want_poll_type;
    
    if (input)
    {
        poll_type = POLLIN;
        want_poll_type = TLS_WANT_POLLIN;
        wants_poll = &want_pollin;
    }
    else
    {
        poll_type = POLLOUT;
        want_poll_type = TLS_WANT_POLLOUT;
        wants_poll = &want_pollout;
    }
    
    int c   = 0;
    int rc  = 0;
    pollfd pfd[1];
    pfd[0].fd = sock;
    pfd[0].events = poll_type;
    int nready = 0;
    
    while(c < timeout || timeout < 0)
    {
        if(*wants_poll)
        {
#ifdef _WIN32
            nready = WSAPoll(pfd, 1, 0);
#else
            nready = poll(pfd, 1, 0);
#endif
            
            if (nready == -1)
                return -1;
            else if (nready == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
                continue;
            }
            
            if (pfd[0].revents & (POLLERR|POLLNVAL))
                return -1;
        }
        
        if (!*wants_poll || (pfd[0].revents & POLLIN))
        {
            *wants_poll = false;
            rc = (int)tls_handshake(tls);
            
            if (rc == want_poll_type)
            {
                *wants_poll = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
            }
            else if (rc != 0)
                return -1;
            else
                break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (timeout > 0 && c < timeout)
                ++c;
        }
    }
    
    return 0;
}

int tls_socket::init()
{
    if(sock < 0)
    {
        sock = INVALID_SOCKET;
        return -1;
    }

    std::lock_guard<std::recursive_mutex> lock(tls_init_mtx);
    
    // Make sure tls_init() is called before anything else
    get_tls_init();
    
    if(mode_ == SM_SERVER)
    {
        struct tls *tls1 = tls_server();
        
        if(tls1 == nullptr)
            return -1;

        server_config = tls_server_config(cert_file_, key_file_);
        
        if(!server_config)
        {
            tls_close(tls1);
            tls_free(tls1);
            return -1;
        }

        if(tls_configure(tls1, server_config) < 0)
        {
            tls_close(tls1);
            tls_free(tls1);            
            server_config = nullptr;
            return -1;
        }
    
        if(tls_accept_socket(tls1, &tls, (int)sock) < 0)
        {
            tls_close(tls1);
            tls_free(tls1);
            server_config = nullptr;
            return -1;
        }
        
        tls_close(tls1);
        tls_free(tls1);

        if(safe_tls_handshake() != 0)
        {
            tls_close(tls);
            tls_free(tls);
            tls = nullptr;
            server_config = nullptr;
            return -1;
        }
    }
    else
    {
        tls = tls_client();

        if(tls == nullptr)
            return -1;
        
        client_config = tls_client_config();
        
        if(!client_config)
        {
            tls_close(tls);
            tls_free(tls);
            tls = nullptr;
            return -1;
        }
        
        tls_configure(tls, client_config);
        
        if(tls_configure(tls, client_config) < 0)
        {
            tls_close(tls);
            tls_free(tls);
            tls = nullptr;
            client_config = nullptr;
            return -1;
        }
        
        if(tls_connect_socket(tls, (int)sock, hostname.c_str()) < 0)
        {
            tls_close(tls);
            tls_free(tls);
            tls = nullptr;
            client_config = nullptr;
            return -1;
        }
        
        if(safe_tls_handshake(false) != 0)
        {
            tls_close(tls);
            tls_free(tls);
            tls = nullptr;
            client_config = nullptr;
            return -1;
        }
    }
    
    initialized_ = true;
    return 0;
}

