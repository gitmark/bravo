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

#ifndef IO_STREAM_H_
#define IO_STREAM_H_

#include <memory>
#include <mutex>

namespace bravo
{

class io_stream
{
public:
    virtual         ~io_stream      () {}
    virtual int     read            (char *buf, int count, int timeout = -1) = 0;
    virtual int     write           (const char *buf, int count, int timeout = -1) = 0;
    virtual bool    timed_out       () { return false; }
    virtual void    clear_timed_out () {}
    virtual int     read_count      () { return 1; }
    virtual void    clear_read_count() {}
    virtual void    set_bad_data    () {}
    virtual bool    bad_data        () { return false; }
    virtual void    clear_bad_data  () {}
    virtual bool    closed          () { return true; }
    virtual int     close           () { return -1; }
    virtual int     flush           () { return -1; }
    virtual void    lock            () {}
    virtual void    unlock          () {}
    virtual int     timeout         () { return -1; }
};

    
class io_end : public io_stream
{
public:
    io_end(std::shared_ptr<io_stream> in_, std::shared_ptr<io_stream> out_)
    : in(in_), out(out_)  {}
        
    int     read                (char *buf, int count, int timeout = -1)        { return in->read(buf, count, timeout); }
    int     write               (const char *buf, int count, int timeout = -1)  { return out->write(buf, count, timeout); }
    bool    timed_out           () { return in->timed_out();                    }
    void    clear_timed_out     () {        in->clear_timed_out();              }
    int     read_count          () { return in->read_count();                   }
    void    clear_read_count    () {        in->clear_read_count();             }
    void    set_bad_data        () {        in->set_bad_data();                 }
    bool    bad_data            () { return in->bad_data();                     }
    void    clear_bad_data      () {        in->clear_bad_data();               }
    int     close               () {        in->close(); return out->close();   }
    int     flush               () { return out->flush();                       }
    void    lock                () {        out->lock();                        }
    void    unlock              () {        out->unlock();                      }
    int     timeout             () { return in->timeout();                      }
        
    bool closed()
    {
        if (in->closed())
        {
            if (!out->closed())
            {
                out->close();
            }
                
            return true;
        }
        else if (out->closed())
        {
            in->close();
                
            return true;
        }
            
        return false;
    }
        
    std::shared_ptr<io_stream> in;
    std::shared_ptr<io_stream> out;
    std::mutex mtx;
};
    
} // namespace bravo

#endif

