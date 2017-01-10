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

#include <bravo/url.h>
#include <bravo/http_utils.h>

using namespace bravo;

#define C 0  // colon
#define S 1  // space
#define F 2  // forward slash
#define A 3  // at symbol
#define O 4  // other

int url_chars[256] = {
//  0  1  2  3     4  5  6  7     8  9  A  B     C  D  E  F
    
    0, O, O, O,    O, O, O, O,    S, S, S, S,    S, S, O, O,  // 0
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 1
    S, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, F,  // 2
    O, O, O, O,    O, O, O, O,    O, O, C, O,    O, O, O, O,  // 3
    
    A, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 4
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 5
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 6
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 7
    
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O
};

#define N 1

int num_chars[256] = {
//  0  1  2  3     4  5  6  7     8  9  A  B     C  D  E  F
    
    0, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 0
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 1
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 2
    N, N, N, N,    N, N, N, N,    N, N, O, O,    O, O, O, O,  // 3
    
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 4
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 5
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 6
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,  // 7
    
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O,
    O, O, O, O,    O, O, O, O,    O, O, O, O,    O, O, O, O
};

#define S_UNK    0
#define S_PROT1  (S_UNK+1)
#define S_PROT   (S_PROT1+1)
#define S_SL1    (S_PROT+1)
#define S_SL2    (S_SL1+1)
#define S_HST1   (S_SL2+1)
#define S_HST    (S_HST1+1)
#define S_PORT1  (S_HST+1)
#define S_PORT   (S_PORT1+1)
#define S_PTH1   (S_PORT+1)
#define S_PTH    (S_PTH1+1)
#define S_END    (S_PTH+1)

int states[][5] = {
    //      :         space     /       @         other
    {       0,          0,      0,      0,          0 }, // S_UNK   unknown
    {       0,    S_PROT1,  S_SL2,      0,     S_PROT }, // S_PROT1 start
    {   S_SL1,          0, S_PTH1, S_HST1,     S_PROT }, // S_PROT  protocol    http
    {       0,          0,  S_SL2,      0,     S_PORT }, // S_SL1   slash 1     http:
    {       0,          0, S_HST1,      0,          0 }, // S_SL2   slash 2     http:/
    {       0,          0,      0,      0,      S_HST }, // S_HST1  host 1      http://
    { S_PORT1,          0, S_PTH1, S_HST1,      S_HST }, // S_HST   host        http://www.abc.com
    {       0,          0,      0,      0,     S_PORT }, // S_PORT1 port 1      http://www.abc.com:
    {       0,          0, S_PTH1, S_HST1,     S_PORT }, // S_PORT  port        http://www.abc.com:123
    {       0,          0,      0,      0,      S_PTH }, // S_PTH1  path 1      http://www.abc.com/
    {   S_PTH,      S_END,  S_PTH,  S_PTH,      S_PTH }, // S_PTH   path        http://www.abc.com/index.html
    {       0,          0,      0,      0,          0 }  // S_END   end         http://www.abc.com/index.html
};

url::url(const std::string& str) :
valid(false)
{
    parse(str);
}

void url::clear()
{
    protocol.clear();
    user.clear();
    password.clear();
    host.clear();
    port.clear();
    path.clear();
    valid = false;
}

int url::parse(const std::string& str)
{
    std::string item;
    int state = S_PROT1;
    int next_state = 0;
    int n = 0;
    
    for(char c : str)
    {
        n = url_chars[c];
        next_state = states[state][n];
        
        if (next_state != state)
        {
            switch(next_state)
            {
                case S_SL1:
                    protocol = item;
                    item.clear();
                    break;
                    
                case S_PORT:
                    if (state == S_SL1)
                    {
                        host = protocol;
                        protocol.clear();
                        item.clear();
                        break;
                    }
                    break;
                    
                case S_PORT1:
                    host = item;
                    item.clear();
                    break;
                    
                case S_HST1:
                    switch(state)
                    {
                        case S_PROT:
                            user = item;
                            item.clear();
                            break;
                            
                        case S_HST:
                            user = item;
                            item.clear();
                            break;
                            
                        case S_PORT:
                            user = host;
                            password = item;
                            host.clear();
                            item.clear();
                            break;
                            
                        default:
                            item.clear();
                            break;
                    }
                    break;
                    
                case S_PTH1:
                    switch(state)
                    {
                        case S_PORT:
                            port = item;
                            item.clear();
                            break;
                            
                        case S_HST:
                            host = item;
                            item.clear();
                            break;
                            
                        case S_PROT:
                            host = item;
                            item.clear();
                            break;
                            
                        default:
                            item.clear();
                            break;
                    }
                    break;
                    
                case S_END:
                    path = item;
                    item.clear();
                    break;
            }
            
            switch(state)
            {
                case S_PROT1:
                case S_HST1:
                case S_PORT1:
                    item.clear();
                    break;
            }
        }
        
        state = next_state;
        if (!state || state == S_END)
            break;
        
        item += c;
    }
    
    switch(state)
    {
        case S_PTH:
            if (item.size())
                path = item;
            else
                path = "/";
            break;
            
        case S_HST:
            host = item;
            path = "/";
            break;
    }
    
    if(!state)
    {
        clear();
        return -1;
    }
    
    for(char c : port)
    {
        int n = num_chars[c];
        
        if(n != N)
        {
            clear();
            return -1;
        }
    }
    
    if (protocol.size())
    {
        auto m = get_protocols().m;
        if (m.find(protocol) == m.end())
        {
            clear();
            return -1;
        }
    }
    
    valid = true;
    return 0;
}

int url::port_num()
{
    if (!valid)
        return 0;
    
    if (port.size())
        return std::atoi(port.c_str());
    
    if (protocol == "https")
        return 443;
    
    return 80;
}

bool url::secure()
{
    if (!valid)
        return false;
    
    if (protocol == "https")
        return true;
    
    if (protocol == "http")
        return false;
    
    if (port.size())
    {
        int n = (int)std::atol(port.c_str());
        if (n == 443)
            return true;
    }
    
    return false;
}

#undef C
#undef S
#undef F
#undef A
#undef O
#undef N


