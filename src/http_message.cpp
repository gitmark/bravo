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

#include <vector>
#include <string>
#include <bravo/string_utils.h>
#include <bravo/http_message.h>
#include <bravo/http_utils.h>
#include <bravo/socket_stream.h>

using namespace std;

#define MAX_CONTENT_LENGTH 4096

namespace bravo
{
http_message::http_message()
{
    request = false;
    sock = nullptr;
    type = dir_specs::dir_type::text;
}

http_message::~http_message()
{}

/*
int http_message::read_from(std::unique_ptr<base_socket> &p, int timeout)
{
    return read_from(p.get(), timeout);
}
*/

int http_message::write_to(std::unique_ptr<base_socket> &p, int timeout)
{
    return write_to(p.get(), timeout);
}

/*
int http_message::read_from(base_socket *s, int timeout)
{
    socket_stream ss(s);
    int rc = read_from(ss);
    
    if (rc < 0)
        return -1;
    
    if (ss.error())
        return -1;
    
    if ((int)s->error())
    {
        if (s->error() == base_socket::socket_error::timeout)
            s->clear_error();
        else
            return -1;
    }

    sock = s;
    return rc;
}
*/

int http_message::read_from(std::istream &is)
{
    clear();
    int total = 0;
    std::getline(is,request_line);
    bravo::chomp(request_line);
    int len = (int)request_line.size() + 2;
    
    if (!request_line.size())
        return -1;

    total += len;
    vector<string> parts;
    bravo::split(request_line, " ", parts);
    
    if (parts.size() != 3)
        return -1;
    
    request         = true;
    method          = parts[0];
    uri             = parts[1];
    http_version    = parts[2];
    
    if (parse_uri(uri,dir,sub_dir,filename,get_param_str))
        return -1;
    
    if(get_param_str.size())
        parse_param_str(get_param_str,params);
    
    len = bravo::read_all_headers(is, headers);

    if(len < 0)
        return -1;

    total += len;

    if (method == "POST")
    {
        std::getline(is,post_param_str);
        bravo::chomp(post_param_str);
        int len2 = (int)post_param_str.size() + 2;
        total += len2;

        if(!post_param_str.size())
            return -1;
        
        parse_param_str(post_param_str,params);
    }

    if (headers.count("Content-Length"))
    {
        int content_length = std::atoi(headers["Content-Length"].c_str());
        if (content_length < 0 || content_length > MAX_CONTENT_LENGTH)
            return -1;
        
        len = read_fixed_length_string(is, content, content_length);

        if (len != content_length)
            return -1;
        
        total += len;
    }
    else if (headers.count("Transfer-Encoding"))
    {
        vector<string> encodings;
        bravo::split(headers["Transfer-Encoding"], ",", encodings);
        
        bool chunked = false;
        
        for (auto &encoding : encodings)
        {
            if (encoding == "Chunked")
            {
                chunked = true;
                break;
            }
        }
        
        if (chunked)
        {
            int content_length = read_chunked_content(is, content);

            if (content_length < 0)
                return -1;

            total += content_length;
        }
    }
    
    return total;
}

int http_message::write_to(base_socket *s, int timeout)
{
    socket_stream ss(s);
    int rc = write_to(ss);
    
    if (rc)
        return rc;
    
    if (ss.error())
        return -1;
    
    if ((int)s->error())
        return -1;
    
    return 0;
}

int http_message::write_to(std::ostream &s)
{
    int len = 0;
    int total = 0;

    if (request)
    {
        request_line = method + " " + uri + " " + http_version + "\r\n";
        len = write_string(s, request_line);
        
        if (len != request_line.size())
            return -1;

        total += len;
    }
    else
    {
        request_line = http_version + " " + status + "\r\n";
        len = write_string(s, request_line);
        
        if (len != request_line.size())
            return -1;
 
        total += len;
    }
    
    len = write_headers(s, headers);

    if (len < 0)
        return -1;

    total += len;
    bool chunked = false;
    
    if (headers.count("Content-Length"))
    {
        int content_length = 0;
        string length_string = headers["Content-Length"];
        
        if (length_string == "?")
        {
            content_length = (int)content.size();

            if (content_length < 0 || content_length > MAX_CONTENT_LENGTH)
                return -1;

            headers["Content-Length"] = std::to_string(content_length);
        }
        else
        {
            content_length = std::atoi(headers["Content-Length"].c_str());
            
            if (content.size() != content_length)
                return -1;
        }
    }
    else if (headers.count("Transfer-Encoding"))
    {
        vector<string> encodings;
        bravo::split(headers["Transfer-Encoding"], ",", encodings);
        
        for (auto &encoding : encodings)
        {
            if (encoding == "Chunked")
            {
                chunked = true;
                break;
            }
        }
    }
    
    
 //   content = "<html><body>hey</body></html>";
    if (content.size())
    {
        if (chunked)
        {
            len = write_chunk(s, content);
            
            if (len < content.size())
                return -1;
            
            total += len;
            len = write_chunk(s, "");

            if (len < 0)
                return -1;

            total += len;
        }
        else
        {
            len = write_string(s, content);
            
            if (len != content.size())
                return -1;

            total += len;
        }
    }
    
    return total;
}

void http_message::clear()
{
    sock = nullptr;
    request = false;
    type = dir_specs::dir_type::unknown;
    request_line.clear();
    method.clear();
    uri.clear();
    http_version = "HTTP/1.1";
    dir.clear();
    sub_dir.clear();
    filename.clear();
    full_path.clear();
    get_param_str.clear();
    post_param_str.clear();
    status.clear();
    content.clear();
    binary_content.clear();
    headers.clear();
    params.clear();
    app_vars.clear();
}

}
