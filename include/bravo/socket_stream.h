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

// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
// http://stackoverflow.com/questions/21420526/implementing-stdbasic-streambuf-subclass-for-manipulating-input

#ifndef socket_stream_h
#define socket_stream_h

#include <bravo/socket_buf.h>
#include <bravo/vec_buf.h>

namespace bravo
{
// B = std::iostream
// T = socket_buf
// A = ctor input param
template<class B, class T, class A>
class socket_stream_ : public B
{
public:
    explicit socket_stream_(A *sock = nullptr);
    explicit socket_stream_(std::unique_ptr<A> &sock);

    int error();
    void clear_error();
    
private:
    T sb;
};

template<class B, class T, class A>
socket_stream_<B,T,A>::socket_stream_(A *sock)
 : std::ios(0), B(&sb), sb(sock,256)
{}

template<class B, class T, class A>
socket_stream_<B,T,A>::socket_stream_(std::unique_ptr<A> &sock)
: std::ios(0), B(&sb), sb(sock.get(),256)
{}

template<class B, class T, class A>
int socket_stream_<B,T,A>::error()
{
    return sb.error();
}

template<class B, class T, class A>
void socket_stream_<B,T,A>::clear_error()
{
    return sb.clear_error();
}

template<class B, class T, class A>
class binary_stream_ : public B {
public:
    explicit binary_stream_(A &sock);
    explicit binary_stream_(std::unique_ptr<A> &sock);

    std::vector<char> vec();
    int error();
    void clear_error();

private:
    T sb;
};

template<class B, class T, class A>
binary_stream_<B,T,A>::binary_stream_(A &sock)
: std::ios(0), B(&sb), sb(sock,256)
{
}

template<class B, class T, class A>
binary_stream_<B,T,A>::binary_stream_(std::unique_ptr<A> &sock)
: std::ios(0), B(&sb), sb(sock.get(),256)
{
}

template<class B, class T, class A>
std::vector<char> binary_stream_<B,T,A>::vec()
{
    sb.sync();
    return sb.vec();
}

template<class B, class T, class A>
int binary_stream_<B,T,A>::error()
{
    return sb.error();
}

template<class B, class T, class A>
void binary_stream_<B,T,A>::clear_error()
{
    return sb.clear_error();
}

typedef socket_stream_<std::iostream, socket_buf<base_socket, base_socket,
    assign_buf<base_socket,base_socket>>, base_socket> socket_stream;
typedef binary_stream_<std::iostream, socket_buf<vec_buf,std::vector<char>,
    alloc_buf<vec_buf, std::vector<char>>>, std::vector<char>> binary_stream;
}

#endif

