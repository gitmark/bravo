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

#include <string>
#include <vector>
#include <sstream>
#include <bravo/hex.h>

namespace bravo
{
typedef unsigned char byte;

int hex_to_num[] = {
    //  0                   4                   8   9   A   B       C   D   E   F
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 0
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 1
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 2
        0,  1,  2,  3,      4,  5,  6,  7,      8,  9,  0,  0,      0,  0,  0,  0, // 3

        0, 10, 11, 12,     13, 14, 15,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 4
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 5
        0, 10, 11, 12,     13, 14, 15,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 6
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0, // 7

        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,

        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
        0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,      0,  0,  0,  0,
};

char num_to_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

std::string char_to_hex(char b)
{
    char buf[3];
    buf[0] = num_to_hex[((unsigned char)b) >> 4];
    buf[1] = num_to_hex[0x0F & b];
    buf[2] = 0;
    return buf;
}

char hex_to_char(const std::string& hex)
{
    if (hex.size() != 2)
        return 0;
    
    int num = hex_to_num[hex[0]];
    num <<= 4;
    num += hex_to_num[hex[1]];
    return (char)num;
}

std::vector<byte> hex_to_vec(const std::string str)
{
    std::vector<byte> result;
    int num = 0;
    int count = str.size() % 2;
    
    for (char c : str)
    {
        if (!count)
        {
            num = hex_to_num[c];
            count = 1;
        }
        else
        {
            num = num << 4;
            num += hex_to_num[c];
            result.push_back(num);
            count = 0;
        }
    }
    
    return move(result);
}

std::string vec_to_hex(const std::vector<byte> &vec)
{
    std::stringstream ss;
    
    for (byte b : vec)
    {
        unsigned msb = (0xF0 & b) >> 4;
        ss << num_to_hex[msb];
        unsigned lsb = (0x0F & b);
        ss << num_to_hex[lsb];
    }
    
    return move(ss.str());
}

}

