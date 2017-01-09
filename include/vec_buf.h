/*****************************************************************************
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

#ifndef vec_buf_h
#define vec_buf_h

#include <vector>
#include <deque>
#include <memory>
#include <mutex>

namespace bravo
{
class vec_buf
{
public:
    vec_buf(int block_size = 4096, int timeout = 1000) :
    read_index(0),
    write_index(0),
    block_size_(block_size),
    timeout_(timeout),
    error_(0)
    {}
    
    vec_buf(const std::vector<char> &vec, int block_size = 4096, int timeout = 1000);
    
    int     write       (const char *buf, int count);
    int     read        (char *buf, int count);
    int     read_to_vec (std::vector<char> &vec);
    int     total_count ();
    bool    eof         ();
    std::vector<char> vec();
    
    inline int  error       () { return error_; }
    inline void clear_error () { error_ = 0; }
    inline int  timeout     () { return timeout_; }
    inline void set_timeout (int timeout) { timeout_ = timeout; }
    
private:
    volatile int            read_index;
    volatile int            write_index;
    const int               block_size_;
    int                     timeout_;
    int                     error_;
    std::recursive_mutex    mtx;
    std::deque<std::unique_ptr<std::vector<char>>> q;
};

}

#endif /* vec_buf_h */
