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

// Good references:
// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
// http://stackoverflow.com/questions/21420526/implementing-stdbasic-streambuf-subclass-for-manipulating-input

#ifndef socket_buf_h
#define socket_buf_h

#include <cstring>
#include <algorithm>

namespace bravo
{
template <class P, class S>
class alloc_buf
{
public:
    static P *create_buf(S *s)
    {
        return new P(s);
    }
    
    static const bool own;
};

template <class P, class S>
const bool alloc_buf<P,S>::own = true;

template <class P, class S>
class assign_buf
{
public:
    static P *create_buf(S *s)
    {
        return s;
    }
    
    static const bool own;
};

template <class P, class S>
const bool assign_buf<P,S>::own = false;

template<class P, class S, class A>
class socket_buf : public std::streambuf
{
public:
    explicit socket_buf(S *sock_, size_t buff_sz = 256, std::size_t put_back = 8) :
    put_back_(std::max(put_back, size_t(1))),
    in_buffer_(std::max(put_back_, buff_sz)),
    out_buffer_(buff_sz),
    error_(0)
    {
        sock = A::create_buf(sock_);
        char *base = &out_buffer_.front();
        setp(base,base + out_buffer_.size() - 1);
        char *end = &in_buffer_.front() + in_buffer_.size();
        setg(end, end, end);
    }
    
    virtual ~socket_buf()
    {
        sync();
    }
    
    std::vector<char> vec()
    {
        return std::move(sock->vec());
    }

    std::streambuf::int_type underflow()
    {
        // In case we are reading and writing to the same buffer, we want to make sure that we flush what
        // we wrote so that we can read it now.
        sync();
        
        if (gptr() < egptr()) // buffer not exhausted
            return traits_type::to_int_type(*gptr());
        
        char *base = &in_buffer_.front();
        char *start = base;
        
        if (eback() == base) // If true, this is not the first fill
        {
            std::memmove(base, egptr() - put_back_, put_back_);
            start += put_back_;
        }
        
        int n = sock->read(start, (int)(in_buffer_.size() - (start - base)));
        
        if (n <= 0)
            return traits_type::eof();
        
        setg(base, start, start + n);        
        return traits_type::to_int_type(*gptr());
    }

    std::streambuf::int_type overflow(std::streambuf::int_type ch)
    {
        if (ch != traits_type::eof())
        {
            *pptr() = ch;
            pbump(1);
            if (do_flush())
                return ch;
        }
        
        return traits_type::eof();
    }

    int do_flush()
    {
        std::ptrdiff_t n = pptr() - pbase();
        
        if (n <= 0)
            return 0;
        
        pbump(-(int)n);
        char *buf = pbase();
        sock->write(buf, (int)n);
        return (int)n;
    }
    
    int sync()
    {
        return do_flush() ? 0 : -1;
    }

    int error()
    {
        return error_;
    }
    
    void clear_error()
    {
        error_ = 0;
    }
    
private:
    // Dont't allow copy or assignment
    socket_buf(const socket_buf &);
    socket_buf &operator= (const socket_buf &);
    
private:
    P *                 sock;
    const size_t        put_back_;
    std::vector<char>   in_buffer_;
    std::vector<char>   out_buffer_;
    int                 error_;
};

}

#endif /* socket_buf_h */
