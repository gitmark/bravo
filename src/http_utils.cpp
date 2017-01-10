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

#include <bravo/string_utils.h>
#include <bravo/http_utils.h>
#include <bravo/socket_stream.h>
#include <bravo/hex.h>

using namespace std;
namespace bravo
{

static int path_chars[] = {
//  0           4           8 9 A B     C D E F
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 1
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,1,4,5, // 2  // -./
    2,2,2,2,    2,2,2,2,    2,2,0,0,    0,0,0,0, // 3  // 0-9
    
    0,3,3,3,    3,3,3,3,    3,3,3,3,    3,3,3,3, // 4  // A-Z
    3,3,3,3,    3,3,3,3,    3,3,3,0,    0,0,0,6, // 5  // _
    0,3,3,3,    3,3,3,3,    3,3,3,3,    3,3,3,3, // 6  // a-z
    3,3,3,3,    3,3,3,3,    3,3,3,0,    0,0,0,0, // 7
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0
};

int check_path_chars(const std::string& path)
{
    for (char c: path)
    {
        if (!path_chars[c])
            return -1;
    }
    
    if (path.find("..") != std::string::npos)
        return -1;
    
    if (path.find("//") != std::string::npos)
        return -1;
    
    return 0;
}

int write_headers(base_socket *s, const map<string, string> &headers, int timeout)
{
    // Return negative if all headers are not written.
    socket_stream ss(s);
    int rc = write_headers(ss, headers);
    
    if (ss.error())
        return -1;
    
    if ((int)s->error())
        return -1;
    
    return rc;
}

int write_headers(std::ostream &s, const std::map<std::string, std::string> &headers)
{
    int total = 0;
    int write_count = 0;

    for (auto &pair : headers)
    {
        // write_string fails if the complete string is not written.
        write_count = write_string(s, pair.first + ": " + pair.second + "\r\n");

        if (write_count < 0)
            return -1;

        total += write_count;
    }
    
    write_count = write_string(s, "\r\n");

    if (write_count != 2)
        return -1;

    total += write_count;
    return total;
}

int read_chunk_length(std::istream &s, int &len)
{
    string len_string;
    std::getline(s, len_string);
    int line_length = (int)s.gcount();
    bravo::chomp(len_string);
    
    if (line_length < 1)
        return -1;
    
    stringstream ss;
    ss << std::hex << len_string;
    ss >> len;
    return line_length;
}

int read_chunk_length(base_socket *s, int &len, int timeout)
{
    string len_string;
    int line_length = s->read_line(len_string);
    
    if (line_length < 1)
        return -1;
    
    stringstream ss;
    ss << std::hex << len_string;
    ss >> len;
    return line_length;
}

#define MAX_FIXED_LENGTH 4096

int read_fixed_length_string(base_socket *s, string &str, int len, int timeout)
{
    str.clear();
    
    if (len == 0)
        return 0;
    
    if (len < 0 || len > MAX_FIXED_LENGTH)
        return -1;
    
    vector<char> buf(len + 1);
    int32_t actual = s->read(buf.data(), len, timeout);
    
    if (actual != len)
        return -1;

    buf[actual] = 0;
    str = buf.data();
    return actual;
}

int read_fixed_length_string(std::istream &s, string &str, int len)
{
    str.clear();
    
    if (len == 0)
        return 0;
    
    if (len < 0 || len > MAX_FIXED_LENGTH)
        return -1;
    
    vector<char> buf(len + 1);
    s.read(buf.data(), len);
    int actual = (int)s.gcount();
    
    if (actual != len)
        return -1;
    
    buf[actual] = 0;
    str = buf.data();
    return actual;
}

int read_chunked_content(std::istream &s, string &str)
{
    str.clear();
    int len = 0;
    string chunk_string;
    stringstream ss;
    string crlf;

    do
    {
        // If read_chunk_length() doesn't read the complete chunk length, it returns negative.
        if (read_chunk_length(s, len) < 0)
            return -1;

        if (len < 0)
            return -1;
        else if (len == 0)
            chunk_string = "";
        else if (read_fixed_length_string(s, chunk_string, len) < 0)
                return -1;

        if (read_fixed_length_string(s, crlf, 2) < 0)
            return -1;

        if (crlf != "\r\n")
            return -1;
        
        if (len > 0)
            ss << chunk_string;
        
    } while (len > 0);
    
    str = ss.str();
    return (int)str.size();
}

int read_chunked_content(base_socket *s, string &str, int timeout)
{
    str.clear();
    int len = 0;
    string chunk_string;
    stringstream ss;
    string crlf;
    
    do
    {
        // If read_chunk_length() doesn't read the complete chunk length, it returns negative.
        if (read_chunk_length(s, len, timeout) < 0)
            return -1;
        
        if (len < 0)
            return -1;
        else if (len == 0)
            chunk_string = "";
        else if (read_fixed_length_string(s, chunk_string, len, timeout) < 0)
                return -1;
        
        if (read_fixed_length_string(s, crlf, 2, timeout) < 0)
            return -1;
        
        if (crlf != "\r\n")
            return -1;
        
        if (len > 0)
            ss << chunk_string;
        
    } while (len > 0);
    
    str = ss.str();
    return (int)str.size();
}

int write_chunk_length(base_socket *s, int len, int timeout)
{
    if (len < 0)
        return -1;
    
    stringstream ss;
    ss << std::hex << len << "\r\n";
    int str_len = (int)ss.str().size();

    if (s->write(ss.str().c_str(), str_len) != str_len)
        return -1;
    
    return str_len;
}

int write_chunk_length(std::ostream &s, int len)
{
    if (len < 0)
        return -1;
    
    stringstream ss;
    ss << std::hex << len << "\r\n";
    int str_len = (int)ss.str().size();
    s.write(ss.str().c_str(), str_len);
    return str_len;
}

int write_string(base_socket *s, const string &str, int timeout)
{
    int len = (int)str.size();
    
    if (len && s->write(str.c_str(), len, timeout) != len)
        return -1;
    
    return len;
}

int write_string(std::ostream &os, const string &str)
{
    os << str;
    os.flush();
    return (int)str.size();
}

int write_chunk(base_socket *s, const string &str, int timeout)
{
    int len = (int)str.size();
    
    if (write_chunk_length(s, len, timeout) < 1)
        return -1;
 
    if (len && write_string(s, str) != str.size())
        return -1;
    
    if (write_string(s, "\r\n") != 2)
        return -1;

    return 0;
}

int write_chunk(std::ostream &s, const string &str)
{
    int len = (int)str.size();
    
    if (write_chunk_length(s, len) < 1)
        return -1;
    
    if (len && write_string(s, str) != str.size())
        return -1;

    if (write_string(s, "\r\n") != 2)
        return -1;
    
    return 0;
}

int write_chunk(base_socket *s, const vector<char> &buf, int timeout)
{
    int len = (int)buf.size();

    if (write_chunk_length(s, len) < 1)
        return -1;

    if (len && s->write(buf.data(), (int)buf.size()) != (int)buf.size())
        return -1;

    if (write_string(s, "\r\n") != 2)
        return -1;

    return 0;
}

int write_chunk(std::ostream &s, const vector<char> &buf)
{
    int len = (int)buf.size();
    
    if (write_chunk_length(s, len) < 1)
        return -1;
    
    if (len)
        s.write(buf.data(), (int)buf.size());
    
    if (write_string(s, "\r\n") != 2)
        return -1;
    
    return len;
}

int write_chunk(base_socket *s, const char *buf, int count, int timeout)
{
    if (!buf)
        return -1;
    
    if (count < 0)
        return -1;

    if (write_chunk_length(s, count, timeout) < 1)
        return -1;

    if (count > 0 && s->write(buf, count, timeout) != (int)count)
        return -1;
    
    if (write_string(s, "\r\n", timeout) != 2)
        return -1;

    return count;
}

int write_chunk(std::ostream &s, const char *buf, int count)
{
    if (!buf)
        return -1;

    if (count < 0)
        return -1;
    
    if (write_chunk_length(s, count) < 1)
        return -1;
    
    if (count > 0)
        s.write(buf, count);
    
    if (write_string(s, "\r\n") != 2)
        return -1;
    
    return count;
}

int get_dir_and_filename(const string &uri, string &dir, string &sub_dir, string &filename)
{
    vector<string> parts;
    bravo::split(uri, "/", parts);
    
    if (parts.size() < 2)
    {
        dir = "";
        sub_dir = "";
        filename = "";
        return -1;
    }
    else
    {
        if (parts[0].size())
        {
            dir = "";
            sub_dir = "";
            filename = "";
            return -1;
        }

        int file_index = (int)parts.size() - 1;
        
        if (file_index > 1)
            dir = parts[1];
        else
            dir = "";
        
        sub_dir = "";
        
        for (int i = 2; i < file_index; i++)
        {
            if (sub_dir.size())
                sub_dir += "/";
            
            sub_dir += parts[i];
        }
        
        filename = parts[file_index];
    }
    
    return 0;
}

std::string replace_all(const std::string& str, const std::string &old_substr, const std::string& new_substr)
{
    std::stringstream ss;
    size_t start = 0;
    size_t pos;
    size_t len;
    
    while (start < str.size())
    {
        pos = str.find(old_substr, start);
        
        if (pos == std::string::npos)
        {
            ss << str.substr(start);
            break;
        }
        
        len = pos - start;
        ss << str.substr(start, len);
        ss << new_substr;
        start = pos + old_substr.size();
    }
    
    return move(ss.str());
}

int parse_uri(const string &uri, string &dir, string &sub_dir, string &filename, string &params)
{
    size_t pos = uri.find("?");
    string path;
    
    if (pos == std::string::npos)
        path = uri;
    else
    {
        path = uri.substr(0,pos);
        
        if (pos < uri.size() - 1)
        {
            size_t len2 = uri.size() - 1 - pos;
            params = uri.substr(pos+1,len2);
        }
    }
    
    if (!path.size())
    {
        dir.clear();
        sub_dir.clear();
        filename.clear();
        params.clear();
        return -1;
    }
    
    int rc = check_path_chars(path);
    
    if (rc)
    {
        dir.clear();
        sub_dir.clear();
        filename.clear();
        params.clear();
        return -1;
    }
    
    if (get_dir_and_filename(path, dir, sub_dir, filename))
    {
        dir.clear();
        sub_dir.clear();
        filename.clear();
        params.clear();
        return -1;
    }
    
    return 0;
}

int parse_pair(const std::string& str, const std::string &delim, std::string &first, std::string &second)
{
    size_t pos1 = str.find(delim);
    
    if (pos1 == std::string::npos)
    {
        first.clear();
        second.clear();
        return -1;
    }
    
    first = str.substr(0, pos1);
    size_t pos2 = pos1 + delim.size();
    size_t len2 = str.size() - pos2;
    
    if (len2)
        second = str.substr(pos2,len2);
    else
        second.clear();
    
    return 0;
}

void parse_param_str(const std::string& param_str, std::map<std::string, std::string> &params)
{
    using namespace bravo;
    std::string name;
    std::string val;
    std::vector<std::string> pairs;
    split(param_str, "&", pairs);
    
    for (std::string &str : pairs)
    {
        parse_pair(str,"=",name,val);
        
        if (params.count(name))
            params[name] += "," + url_decode(val);
        else
            params[name] = url_decode(val);
    }
}

int read_all_headers(base_socket *s, std::map<std::string, std::string> &headers, int timeout)
{
    using namespace bravo;
    headers.clear();
    int rc = 0;
    int len = 0;
    int total = 0;
    std::string line;
    std::string name;
    std::string val;
    
    while (1)
    {
        len = s->read_line(line);
        
        if (len == 2)
        {
            // Read empty line, end of headers
            total += len;
            break;
        }
        else if (len < 2)
        {
            headers.clear();
            return -1;
        }
        
        total += len;
        chomp(line);
        rc = parse_pair(line, ":", name, val);
        
        if (rc)
        {
            headers.clear();
            return -1;
        }
        
        if (headers.count(name))
            headers[name] += "," + val;
        else
            headers[name] = val; 
    }
    
    return total;
}

int parse_header1(const std::string &line, std::string &name, std::string &val)
{
    size_t pos = line.find(':');
    
    if (pos == std::string::npos)
    {
        name = "";
        val = "";
        return -1;
    }
    
    size_t name_len = pos;
    size_t val_len = line.size() - pos - 1;
    
    if (!name_len)
    {
        name = "";
        val = "";
        return -1;
    }
    
    name = bravo::trim(line.substr(0, name_len));
    
    if (val_len)
        val = bravo::trim(line.substr(pos + 1, val_len));
    else
        val = "";
    return 0;
}

protocols &get_protocols()
{
    static protocols prot;
    return prot;
}

// http://stackoverflow.com/questions/1812473/difference-between-url-encode-and-html-encode

/* URLEncode
 
 Returns a string in which all non-alphanumeric characters except -_. have been replaced with a percent (%) sign followed by two hex digits and spaces encoded as plus (+) signs. It is encoded the same way that the posted data from a WWW form is encoded, that is the same way as in application/x-www-form-urlencoded media type. This differs from the » RFC 1738 encoding (see rawurlencode()) in that for historical reasons, spaces are encoded as plus (+) signs. */

int url_encode_chars[256] = {
//  0  1  2  3     4  5  6  7     8  9  A  B     C  D  E  F
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,  // 0
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,  // 1
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 1, 1, 0,  // 2  - .
    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 0, 0,    0, 0, 0, 0,  // 3  0-9
    
    0, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,  // 4 A-Z
    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 0,    0, 0, 0, 1,  // 5 _
    0, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 1,  // 6 a-z
    1, 1, 1, 1,    1, 1, 1, 1,    1, 1, 1, 0,    0, 0, 0, 0,  // 7
    
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
};

std::string url_encode(const std::string &url)
{
    std::stringstream ss;

    for (char c : url)
    {
        int n = url_encode_chars[c];
        
        if (n == 1)
            ss << c;
        else if (c == ' ')
            ss << '+';
        else
        {
            ss << '%';
            ss << char_to_hex(c);
        }
    }
    
    return move(ss.str());
}

std::string url_decode(const std::string &url)
{
    char buf[3];
    buf[2] = 0;
    std::stringstream ss;
    int state = 0;
    
    for (char c : url)
    {
        switch(state)
        {
            case 0:
                if (c == '%')
                    state = 1;
                else
                    ss << c;
                break;
                
            case 1:
                buf[0] = c;
                state = 2;
                break;
                
            case 2:
                buf[1] = c;
                state = 0;
                ss << hex_to_char(buf);
                break;
        }
    }
    
    return move(ss.str());
}

std::string get_file_ext(const std::string &filename)
{
    size_t pos = filename.find_last_of("./");
    
    if (pos == std::string::npos)
        return "";
    
    if (filename[pos] == '/')
        return "";
    
    if (pos == filename.size() - 1)
        return "";
    
    return filename.substr(pos + 1);
}

}

