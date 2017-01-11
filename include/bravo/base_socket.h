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

#ifndef base_socket_h
#define base_socket_h

#ifndef _WIN32
    #define SOCKET int
    #include <netdb.h>
#else
    #define NOMINMAX
    #include <winsock2.h>
#endif

#include <string>

namespace bravo
{
void* init_sockets();

static constexpr const char *socket_error_strings_[] = {    "none", "unknown", "already open", "init", "socket",
                                                            "bad_index", "timeout", "bad_param", "closed", "max_error"};
class base_socket
{
public:
    const static int    default_timeout_ = 2000;
    const static int    error_start_ = -1000;
    enum class          socket_error {  none = 0, unknown = error_start_, already_open, init, socket,
                                        bad_index, timeout, bad_param, closed, max_error };
    base_socket() { error_ = socket_error::none; init_sockets(); }
    virtual         ~base_socket() {};
    
    virtual int     read_       (char *buf, int count) = 0;
    virtual int     read        (char *buf, int count, int timeout = default_timeout_) = 0;
    virtual int     read        (char *buf, int count, const char *end, int end_len, int timeout = default_timeout_) = 0;
    virtual int     read_line   (std::string &str, const int buf_size = default_read_line_buf_size) = 0;
    virtual int     write_      (const char *buf, int count) = 0;
    virtual int     write       (const char *buf, int count, int timeout = default_timeout_) = 0;
    virtual int     write       (const std::string &str) = 0;
    virtual int     listen      (short port_) = 0;
    virtual SOCKET  accept      (int timeout = default_timeout_) = 0;
    virtual int     connect     (const std::string &hostname, short port_, int timeout = default_timeout_) = 0;
    virtual int     connect     (const std::string &hostname, in_addr &ip, short port_, int timeout = default_timeout_) = 0;
    virtual int     close       () = 0;
    virtual bool    closed      () = 0;
    virtual int     set_no_delay() = 0;
    virtual int     init        () = 0;
    virtual bool    initialized () = 0;
    virtual socket_error error  () { return error_; }
    virtual void    clear_error () { error_ = socket_error::none; }
    
    static std::string error_string(socket_error e);
    static std::string error_string(int e);

protected:
    static const int    default_read_line_buf_size = 10000;
    socket_error        error_;
};
    
}

#endif /* base_socket_h */
    
