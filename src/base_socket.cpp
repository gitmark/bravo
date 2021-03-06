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

#include <bravo/base_socket.h>

namespace bravo
{
#ifdef _WIN32
class WSInit {
public:
    WSInit() { WSAStartup(MAKEWORD(2, 2), &wsaData); }
    ~WSInit() { WSACleanup(); }
    WSAData wsaData;
};

void* init_sockets()
{
    static WSInit wsinit;
    return &wsinit;
}
#else
void* init_sockets()
{
    return nullptr;
}
#endif
    

std::string base_socket::error_string(socket_error e)
{
    return error_string((int)e);
}

std::string base_socket::error_string(int e)
{
    if(!e)
        return socket_error_strings_[e];
    
    e -= error_start_;
    
    if (e < 1 && e >= ((int)socket_error::max_error - error_start_))
        e = 0;
    
    return socket_error_strings_[e];
}

}

