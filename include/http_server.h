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

#ifndef http_server_h
#define http_server_h

#include <vector>
#include <memory>
#include <bravo/base_server.h>
#include <bravo/http_listen_port.h>

namespace bravo
{
template<class T>
std::unique_ptr<T> to_unique(T *p)
{
    std::unique_ptr<T> ptr(p);
    return std::move(ptr);
}

class http_server : public base_server
{
public:
    http_server() {}

    ~http_server()
    {
        stop();
    }
    
    void add_port(http_listen_port *port)
    {
        ports.push_back(std::move(unique_ptr<http_listen_port>(port)));
    }
    
    void start()
    {
        for(auto &p : ports)
            p->start();
    }
    
    void stop()
    {
        for(auto &p : ports)
            p->set_stop();
        
        for(auto &p : ports)
        {
            p->stop();
            p->close();
        }
    }

    std::vector<std::unique_ptr<http_listen_port>> ports;
};

}

#endif /* http_server_h */


