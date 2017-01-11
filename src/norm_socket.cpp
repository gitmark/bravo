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
        #define _WIN32_WINNT 0x0700
    #endif
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

#ifdef __linux__
    #include <stropts.h>
#endif

#include <algorithm>
#include <functional>
#include <cmath>
#include <thread>
#include <bravo/norm_socket.h>
#include <bravo/socket_utils.h>

using namespace std;

#define BACKLOG 20

namespace bravo
{
int data_match(const char *data, int len, const char *end_marker, int end_len, int &mcount);
bool is_ip(const std::string &str);

norm_socket::norm_socket()
{
    clear();
}

norm_socket::norm_socket(SOCKET s)
{
    clear();
    sock = s;
}

int norm_socket::init()
{
    if (initialized_)
        return 0;

    int on = 1; 
    int rc = ioctlsocket(sock, FIONBIO, (u_long*)&on);   
    if (SOCKET_ERROR == rc)
        return -1;

    initialized_ = true;
    return 0;
}

void norm_socket::clear()
{
    hostname.clear();
    port = 0;
    sock = INVALID_SOCKET;
    buf_.clear();
    cur_ = nullptr;
    end_ = nullptr;
    want_pollin = true;
    want_pollout = false;
    mode_ = SM_SERVER;
    initialized_ = false;
}

int norm_socket::read_(char *buf, int count)
{
    int actual = (int)recv(sock, buf, count, 0);
    
    if (actual < 0 && WSAEWOULDBLOCK == WSAGetLastError())
        return SOCKET_WANT_POLLIN;
    
    return actual;
}

int norm_socket::write_(const char *buf, int count)
{
    int actual = (int)send(sock, buf, count, 0);
    
    if (actual < 0 && WSAEWOULDBLOCK == WSAGetLastError())
        return SOCKET_WANT_POLLOUT;
    
    return actual;
}

int norm_socket::read(char *buf, int count, const char *end, int end_len, int timeout)
{
    if (!buf || count < 0 || end_len < 0 || timeout < -1)
    {
        error_ = socket_error::bad_param;
        return (int)error_; // Return bad param error
    }
    
    if (count == 0)
        return 0;  // Return nothing read, no error
    
    char *b         = buf;
    int remaining   = count;
    int actual      = 0;
    int c           = 0;
    int mcount      = 0; // Start fresh, no previous match history
    
    // Check if there is data cached from the last read
    if (cur_ < end_)
    {
        int next_index = 0;
        int len = (int)(end_ - cur_);
        
        if (end_len)
            next_index = data_match(cur_, len, end, end_len, mcount);
        
        if (end_len && mcount == end_len)
            actual = std::min(remaining,next_index);
        else
            actual = std::min(remaining, len);
        
        memcpy(b, cur_, actual);
        b += actual;
        cur_ += actual;
        remaining -= actual;
        
        if (!remaining || mcount == end_len)
            return actual;
    }
    
    if (closed())
    {
        if (b != buf)
            return (int)(b - buf);
        
        error_ = socket_error::closed;
        return (int)error_; // Return closed error
    }
    
    pollfd pfd[1];
    pfd[0].fd = sock;
    pfd[0].events = POLLIN;
    
    int nready = 0;
    
    while (c < timeout || timeout < 0)
    {
        if (want_pollin)
        {
#ifdef _WIN32
            nready = WSAPoll(pfd, 1, 0);
#else
            nready = poll(pfd, 1, 0);
#endif
            if (nready == -1)
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
            else if (nready == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
                continue;
            }
            
            if (pfd[0].revents & (POLLERR|POLLNVAL))
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
        }
        
        if (!want_pollin || (pfd[0].revents & POLLIN))
        {
            want_pollin = false;
            
            if (!initialized_)
            {
                if (this->init())
                {
                    error_ = socket_error::init;
                    return (int)error_; // Return init error
                }
            }
            
            actual = read_(b, remaining);
            
            if (actual == SOCKET_WANT_POLLIN)
            {
                want_pollin = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
            }
            else if (actual < 0)
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
            else if (actual == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;

//                close();
//                error_ = socket_error::closed;
//                return (int)error_; // Return close error
            }
            else
            {
                int next_index = 0;
                
                if (end_len)
                    next_index = data_match(b, actual, end, end_len, mcount);
                
                if (end_len && mcount == end_len)
                {
                    int len = next_index;
                    int rem = actual - len;
                    char *next = b + len;
                    buf_.resize(rem);
                    memcpy(buf_.data(), next, rem);
                    cur_ = buf_.data();
                    end_ = cur_ + rem;
                    
                    b += len;
                    remaining -= len;
                    break;
                }
                else
                {
                    b += actual;
                    remaining -= actual;
                    if (!remaining)
                        break;
                }
                
                if (timeout > 0)
                {
                    // Timeout applies to the start of the message. After we start receiving the message the
                    // time remaining drops to 100 ms
                    c = timeout - 100;
                    if (c < 0)
                        c = 0;
                }
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (timeout > 0 && c < timeout)
                ++c;
        }
    }
    
    if (b != buf)
        return (int)(b - buf); // No error, return count
    
    
    if (c == timeout)
    {
        return 0;
//        error_ = socket_error::timeout;
//        return (int)error_; // Return timeout error
    }
        
    return 0;
}

int norm_socket::read(char *buf, int count, int timeout)
{
    return read(buf, count, nullptr, 0, timeout);
}

int norm_socket::read_line(std::string &str, int buf_size)
{
    vector<char> buf(buf_size + 1);
    int r = read(buf.data(), buf_size, "\r\n", 2);

    if (r >= 0)
    {
        buf[r] = 0;
        str = buf.data();
    }
    else
        str.clear();
    
    return r;
}

int norm_socket::write(const char *buf, int count, int timeout)
{
    if (!buf || count < 0 || timeout < -1)
    {
        error_ = socket_error::bad_param;
        return (int)error_; // Return bad param error
    }

    if (count == 0)
        return 0;  // Nothing written, return no error
    
    if (closed())
    {
        error_ = socket_error::closed;
        return (int)error_; // Return closed error
    }
    
    pollfd pfd[1];
    pfd[0].fd = sock;
    pfd[0].events = POLLOUT;
    const char *b = buf;
    int remaining = count;
    int actual = 0;
    int c = 0;
    int nready = 0;

    while (c < timeout || timeout < 0)
    {
        if (want_pollout)
        {
#ifdef _WIN32
            nready = WSAPoll(pfd, 1, 0);
#else
            nready = poll(pfd, 1, 0);
#endif
            if (nready == -1)
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
            else if (nready == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
                continue;
            }
            
            if (pfd[0].revents & (POLLERR|POLLNVAL))
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
        }
        
        if (!want_pollout || (pfd[0].revents & POLLOUT))
        {
            want_pollout = false;

            if (!initialized_)
            {
                if (this->init())
                {
                    error_ = socket_error::init;
                    return (int)error_; // Return init error
                }
            }
            
            actual = write_(b, remaining);
            
            if (actual == SOCKET_WANT_POLLOUT)
            {
                want_pollout = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (timeout > 0 && c < timeout)
                    ++c;
            }
            else if (actual < 0)
            {
                close();
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
            else if (actual == 0)
            {
                close();
                error_ = socket_error::closed;
                return (int)error_; // Return close error
            }
            else
            {
                b += actual;
                remaining -= actual;
                if (!remaining)
                    break;
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (timeout > 0 && c < timeout)
                ++c;
        }
    }
    
    if (b != buf)
        return (int)(b - buf); // No error, return count
    
    if (c == timeout)
    {
        error_ = socket_error::timeout;
        return (int)error_; // Return timeout error
    }
    
    return 0;
}

int norm_socket::write(const std::string &str)
{
    return write(str.c_str(), (int)str.size());
}

int norm_socket::connect(const std::string &hostname, short port_, int timeout)
{
    in_addr addr;
    
    if (is_ip(hostname))
        inet_pton(AF_INET, hostname.c_str(), &(addr.s_addr));
    else
    {
        hostent *host = gethostbyname(hostname.c_str());
#ifdef _WIN32
        addr.s_addr = *(ULONG*)(host->h_addr);
#else
        addr.s_addr = *(in_addr_t*)(host->h_addr);
#endif
    }
    
    return connect(hostname, addr, port_, timeout);
}

int norm_socket::connect(const std::string &hostname_, in_addr &ip_, short port_, int timeout)
{
    if (sock != INVALID_SOCKET)
    {
        error_ = socket_error::already_open;
        return (int)error_; // Return already open error
    }

    mode_       = SM_CLIENT;
    hostname    = hostname_;
    port        = port_;
    int c       = 0;
    SOCKET server_sock = INVALID_SOCKET;
    
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ip_.s_addr;
    
    while (c < timeout || timeout < 0)
    {
        if (::connect(server_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
        {
            if (WSAEWOULDBLOCK == WSAGetLastError())
            {
                if (timeout >= 0)
                {
                    if (c == timeout)
                    {
                        safe_close(server_sock);
                        server_sock = INVALID_SOCKET;
                        error_ = socket_error::timeout;
                        return (int)error_; // Return timeout error
                    }
                    else
                    {
                        if (c < timeout)
                            ++c;
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            else
            {
                safe_close(server_sock);
                server_sock = INVALID_SOCKET;
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
        }
        else
        {
            break;
        }
    }
    
    sock = server_sock;
    return 0;
}

int norm_socket::close()
{
    if (sock != INVALID_SOCKET)
    {
        safe_close(sock);
        sock = INVALID_SOCKET;
    }

    return 0;
}

bool norm_socket::closed()
{
    return (sock == INVALID_SOCKET);
}

norm_socket::~norm_socket()
{
    close();
}

int norm_socket::listen(short port_)
{
    if (sock != INVALID_SOCKET)
    {
        error_ = socket_error::already_open;
        return (int)error_; // Return already open error
    }
    
    port            = port_;
    int on          = 1;
    int rc          = 0;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (INVALID_SOCKET == sock)
    {
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    
    if (SOCKET_ERROR == rc)
    {
        close();
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    rc = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));

    if (SOCKET_ERROR == rc)
    {
        close();
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    sockaddr_in localSockAddr;
    memset(&localSockAddr, 0, sizeof(localSockAddr));
    localSockAddr.sin_family = AF_INET;               // Address family
    localSockAddr.sin_addr.s_addr = INADDR_ANY;       // Wild card IP address
    localSockAddr.sin_port = htons(port);             // Port to use
    rc = ::bind(sock, (sockaddr*)&localSockAddr, sizeof localSockAddr);
    
    if (SOCKET_ERROR == rc)
    {
        close();
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    rc = ::listen(sock, BACKLOG);
    
    if (SOCKET_ERROR == rc)
    {
        close();
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    rc = ioctlsocket(sock, FIONBIO, (u_long*)&on);

    if (SOCKET_ERROR == rc)
    {
        close();
        error_ = socket_error::socket;
        return (int)error_; // Return socket error
    }
    
    return 0;
}

SOCKET norm_socket::accept(int timeout)
{
    if (sock == INVALID_SOCKET)
    {
        error_ = socket_error::closed;
        return (int)error_; // Return closed error
    }
    
    int c = 0;
    sockaddr_in    newSockAddr;
    memset(&newSockAddr, 0, sizeof(newSockAddr));
    socklen_t sizeNewSockAddr = sizeof newSockAddr;
    SOCKET newSocket = INVALID_SOCKET;
    
    while (c < timeout || timeout < 0)
    {
        newSocket = ::accept(sock, (sockaddr*)&newSockAddr, &sizeNewSockAddr);
        
        if (newSocket == INVALID_SOCKET)
        {
            if (WSAEWOULDBLOCK == WSAGetLastError())
            {
                if (timeout >= 0)
                {
                    if (c == timeout)
                    {
                        newSocket = INVALID_SOCKET;
                        error_ = socket_error::timeout;
                        return (int)error_; // Return timeout error
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
                if (timeout >= 0 && c < timeout)
                    ++c;
                
                continue;
            }
            else
            {
                error_ = socket_error::socket;
                return (int)error_; // Return socket error
            }
        }
        else
            break;
    }
    
    if (INVALID_SOCKET != newSocket)
    {
        int off = 0;
        int rc = ioctlsocket(newSocket, FIONBIO, (u_long*)&off);
        
        if (SOCKET_ERROR == rc)
        {
            close();
            error_ = socket_error::socket;
            return (int)error_; // Return socket error
        }
    }

    return newSocket;
}

int norm_socket::set_no_delay()
{
    int on = 1;
    return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));
}

}
