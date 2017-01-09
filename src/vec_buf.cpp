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

#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <bravo/vec_buf.h>

using namespace bravo;

vec_buf::vec_buf(const std::vector<char> &v, int block_size, int timeout) :
read_index(0),
write_index(block_size),
block_size_(block_size),
timeout_(timeout),
error_(0)
{
    if (!v.size())
        return;
        
    write(v.data(), (int)v.size());
}

int vec_buf::write(const char *buf, int count)
{
    if (!buf || count < 0)
        return -1;
    
    if (count == 0)
        return 0;
    
    const char *curr = buf;
    int remaining = count;
    int available;
    int actual;
    char *start;
    
    while(remaining)
    {
        available = block_size_ - write_index;

        if (!available)
        {
            std::unique_lock<std::recursive_mutex> lock(mtx);
            q.push_back(std::make_unique<std::vector<char>>(block_size_));
            available = block_size_;
            write_index = 0;
        }
        
        actual = std::min(remaining, available);
        start = &q.back()->at(write_index);
        memcpy(start, curr, actual);
        curr += actual;
        remaining -= actual;
        write_index += actual;
    }
    
    return (int)(curr - buf);
}

int vec_buf::read(char *buf, int count)
{
    if (!buf || count < 0)
        return -1;
    
    if (count == 0)
        return 0;

    int c = 0;
    
    while(eof() && (c < timeout_ || timeout_ < 0))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        if (timeout_ >= 0)
        {
            if (c < timeout_)
                ++c;
            else
            {
                error_ = -1;
                return error_;
            }
        }
    }
    
    char *curr = buf;
    int remaining = count;
    int available;
    int actual;
    char *start;
    std::unique_lock<std::recursive_mutex> lock(mtx);

    while(remaining)
    {
        if (!q.size())
            break;
        else if (1 == q.size())
            available = write_index - read_index;
        else
            available = block_size_ - read_index;
        
        if (!available)
            break;
        
        actual = std::min(remaining, available);
        start = &q.front()->at(read_index);
        memcpy(curr, start, actual);
        curr += actual;
        remaining -= actual;
        read_index += actual;
        
        if (read_index == block_size_)
        {
            q.pop_front();
            read_index = 0;
        }
        else
            break;
    }

    return (int)(curr - buf);
}

bool vec_buf::eof()
{
    if (!q.size())
        return true;
    
    std::unique_lock<std::recursive_mutex> lock(mtx);
    
    if(q.size() == 1 && read_index == write_index)
        return true;
    
    return false;
}

std::vector<char> vec_buf::vec()
{
    std::vector<char> v;
    read_to_vec(v);
    return std::move(v);
}

int vec_buf::read_to_vec(std::vector<char> &v)
{
    std::unique_lock<std::recursive_mutex> lock(mtx);
    int count = total_count();
    
    if (!count)
    {
        v.clear();
        return 0;
    }
    
    int read_bin = 0;
    int read_index2 = read_index;
    v.resize(count);
    char *curr = v.data();
    int remaining = count;
    int available;
    int actual;
    char *start;
    
    while(remaining)
    {
        if (1 == q.size())
            available = write_index - read_index2;
        else
            available = block_size_ - read_index2;
        
        if (!available)
            break;
        
        actual = std::min(remaining, available);
        start = &q[read_bin]->at(read_index2);
        memcpy(curr, start, actual);
        curr += actual;
        remaining -= actual;
        read_index2 += actual;
        
        if (read_index2 == block_size_)
        {
            ++read_bin;
            read_index2 = 0;
        }
        else
            break;
    }
    
    return (int)(curr - v.data());
}

int vec_buf::total_count()
{
    if (!q.size())
        return 0;
    
    std::unique_lock<std::recursive_mutex> lock(mtx);

    if(q.size() == 1)
        return write_index - read_index;
    
    int count = write_index + block_size_ - read_index;
    
    if (q.size() == 2)
        return count;
    
    return count + (int)(q.size() - 2)*block_size_;
}


