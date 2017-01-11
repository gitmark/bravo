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

#ifndef http_message_h
#define http_message_h

#include <string>
#include <map>
#include <bravo/base_socket.h>
#include <bravo/dir_specs.h>

namespace bravo
{
class http_message
{
public:
    http_message();
    virtual ~http_message();
    int     read_from   (std::istream &is_);

    int     write_to    (base_socket *s, int timeout = base_socket::default_timeout_);
    int     write_to    (std::unique_ptr<base_socket> &p, int timeout = base_socket::default_timeout_);
    int     write_to    (std::ostream &os_);
    void    clear();
    
    base_socket *sock;
    bool        request;
    dir_specs::dir_type type;
    std::string request_line;
    std::string method;
    std::string uri;
    std::string http_version;
    std::string dir;
    std::string sub_dir;
    std::string filename;
    std::string full_path;
    std::string get_param_str;
    std::string post_param_str;
    std::string status;
    std::string content;
    std::vector<char> binary_content;
    std::map<std::string,std::string> headers;
    std::map<std::string,std::string> params;
    std::map<std::string,std::string> app_vars;
};
    
}

#endif
