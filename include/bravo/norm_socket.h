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

#ifndef norm_socket_h_
#define norm_socket_h_

#include <string>
#include <vector>
#include <bravo/base_socket.h>

#ifndef _WIN32
#define SOCKET int
#endif

#define SOCKET_WANT_POLLIN  -2
#define SOCKET_WANT_POLLOUT -3
#define SM_SERVER   1
#define SM_CLIENT   2

namespace bravo
{
class norm_socket : public base_socket
    {
    public:
        norm_socket();
        norm_socket(SOCKET s);
        virtual ~norm_socket();
        
        virtual int read_   (char *buf, int count);
        virtual int read    (char *buf, int count, int timeout = default_timeout_);
        virtual int read    (char *buf, int count, const char *end, int end_len, int timeout = default_timeout_);
        virtual int read_line(std::string &str, const int buf_size = default_read_line_buf_size);
        virtual int write_  (const char *buf, int count);
        virtual int write   (const char *buf, int count, int timeout = default_timeout_);
        virtual int write   (const std::string &str);
        virtual int listen  (short port_);
        virtual SOCKET accept(int timeout = default_timeout_);
        virtual int connect (const std::string &hostname, short port_, int timeout = default_timeout_);
        virtual int connect (const std::string &hostname, in_addr &ip, short port_, int timeout = default_timeout_);
        virtual int close   ();
        virtual bool closed ();
        virtual int set_no_delay();
        virtual int init    ();
        virtual bool initialized() { return initialized_; }
        virtual void clear  ();

    protected:
        std::string             hostname;
        short                   port;
        SOCKET                  sock;
        std::vector<char>       buf_;
        char*                   cur_;
        char*                   end_;
        bool                    want_pollin;
        bool                    want_pollout;
        int                     mode_;
        bool                    initialized_;
    };
}

#endif // norm_socket_h_

