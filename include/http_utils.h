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

#ifndef http_utils_h
#define http_utils_h

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <bravo/base_socket.h>

namespace bravo
{
class protocols
{
public:
    protocols()
    {
        m["https"];
        m["http"];
    }
    
    std::map<std::string, int> m;
};

int write_headers           (base_socket  *s,   const std::map<std::string, std::string> &headers,
                             int timeout = base_socket::default_timeout_);
int write_headers           (std::ostream &s,   const std::map<std::string, std::string> &headers);
int read_all_headers        (base_socket  *s,   std::map<std::string, std::string> &headers,
                             int timeout = base_socket::default_timeout_);
int read_chunk_length       (base_socket  *s,   int &len, int timeout = base_socket::default_timeout_);
int read_chunk_length       (std::istream &s,   int &len);
int read_fixed_length_string(base_socket  *s,   std::string &str, int len, int timeout = base_socket::default_timeout_);
int read_fixed_length_string(std::istream &s,   std::string &str, int len);
int read_chunked_content    (base_socket  *s,   std::string &str, int timeout = base_socket::default_timeout_);
int read_chunked_content    (std::istream &s,   std::string &str);
int write_chunk_length      (base_socket  *s,   int len, int timeout = base_socket::default_timeout_);
int write_chunk_length      (std::ostream &s,   int len);
int write_string            (base_socket  *s,   const std::string &str, int timeout = base_socket::default_timeout_);
int write_string            (std::ostream &s,   const std::string &str);
int write_chunk             (base_socket  *s,   const std::string &str, int timeout = base_socket::default_timeout_);
int write_chunk             (std::ostream &s,   const std::string &str);
int write_chunk             (base_socket  *s,   const std::vector<char> &buf, int timeout = base_socket::default_timeout_);
int write_chunk             (std::ostream &s,   const std::vector<char> &buf);
int write_chunk             (base_socket  *s,   const char *buf, int count, int timeout = base_socket::default_timeout_);
int write_chunk             (std::ostream &s,   const char *buf, int count);
int parse_uri               (const std::string &uri, std::string &dir, std::string &sub_dir, std::string &filename,
                             std::string &params);
int parse_header            (const std::string &line, std::string &name, std::string &val);
int parse_pair              (const std::string &str, const std::string &delim, std::string &first, std::string &second);
void parse_param_str        (const std::string &param_str, std::map<std::string, std::string> &params);
std::string content_type1   (const std::string &path);
std::string url_encode      (const std::string &url);
std::string url_decode      (const std::string &url);
std::string get_file_ext    (const std::string &filename);
protocols &get_protocols    ();

}

#endif


