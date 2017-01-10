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

#include <bravo/socket_utils.h>

using namespace bravo;

#define CLOSE_BUF_SIZE 1024

#ifndef _WIN32
#include <unistd.h>
#define closesocket close
#define SD_SEND SHUT_RDWR
#define SOCKET_ERROR -1
#endif

int safe_close(SOCKET s)
{
    // Closing sockets gracefully per Microsoft documentaion.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms738547(v=vs.85).aspx
    
    char closeBuf[CLOSE_BUF_SIZE];
    size_t readCount = 1;
    shutdown(s, SD_SEND);

    while (readCount != 0 && readCount != SOCKET_ERROR)
    {
        readCount = recv(s, closeBuf, CLOSE_BUF_SIZE, 0);
    }

    closesocket(s);
    return 0;
}



