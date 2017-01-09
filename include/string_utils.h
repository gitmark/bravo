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

#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <deque>

#include <bravo/io_stream.h>


static int char_types[] = {
//  0           4           8 9 A B     C D E F
    0,0,0,0,    0,0,0,0,    1,1,1,1,    1,1,0,0, // 0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 1
    1,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 2
    2,2,2,2,    2,2,2,2,    2,2,0,0,    0,0,0,0, // 3
    
    0,3,3,3,    3,3,3,3,    3,3,3,3,    3,3,3,3, // 4
    3,3,3,3,    3,3,3,3,    3,3,3,0,    0,0,0,0, // 5
    0,3,3,3,    3,3,3,3,    3,3,3,3,    3,3,3,3, // 6
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


namespace bravo
{
    
    inline void chomp(std::string &str)
    {
        if(!str.size())
            return;
        
        if (str[str.size()-1] == '\r')
        {
            str.resize(str.size()-1);
            return;
        }
        
        if (str[str.size()-1] == '\n')
        {
            if (str.size() > 1 && str[str.size()-2] == '\r')
                str.resize(str.size()-2);
            else
                str.resize(str.size()-1);
        }
    }
    

    inline std::string trim(const std::string &str)
    {
        if (!str.size())
            return std::move(std::string());
        
        int start = 0;
        
        while(char_types[str[start]] == 1)
            ++start;
        
        if(start == str.size())
            return "";

        int stop = (int)str.size() - 1;

        while(char_types[str[stop]] == 1)
            --stop;
        
        return move(str.substr(start, stop - start + 1));
    }
    

    inline std::string trim_left(const std::string &str)
    {
        if (!str.size())
            return std::move(std::string());

        int start = 0;
        
        while(char_types[str[start]] == 1)
            ++start;
        
        if(start == str.size())
            return "";
        
        return move(str.substr(start));
    }
    

    inline std::string trim_right(const std::string &str)
    {
        if (!str.size())
            return std::move(std::string());

        int stop = (int)str.size() - 1;
        
        while(stop >= 0 && char_types[str[stop]] == 1)
            --stop;
        
        if (stop < 0)
            return "";
        
        return move(str.substr(0, stop + 1));
    }
    

    template<class T>
    void split(const std::string &str, char delim, T &vec)
    {
        vec.clear();
    
        if(!str.size())
            return;
    
        size_t start    = 0;
        size_t stop     = 0;
        size_t len      = 0;
    
        while(start < str.size())
        {
            stop = str.find(delim, start);
            if (stop == std::string::npos)
                stop = str.size();
        
            len = stop - start;       
            vec.push_back(str.substr(start,len));
            start = stop + 1;
        }
    
        if (start == str.size())
            vec.push_back("");
    
        return;
    }


    template<class T>
    void to_stream(const T &vec, std::ostream &os)
    {
        int i = 0;
        
        for(auto val : vec)
        {
            if (i > 0)
                os << ", ";
        
            os << val;
            ++i;
        }
    }


    template<class T>
    std::string to_string(const std::vector<T> &vec)
    {
        std::stringstream ss;
        int i = 0;
        
        for(auto val : vec)
        {
            if (i > 0)
                ss << ", ";
        
            ss << val;
            ++i;
        }
    
        return ss.str();
    }


    inline void split(const std::string &str, const std::string &delim, std::vector<std::string> &vec)
    {
        vec.clear();
        
        int len = 0;
        int delim_pos = 0;
        int token_pos = 0;
        
        do
        {
            if (token_pos == str.size())
            {
                delim_pos = (int)str.size();
            }
            else
            {
                delim_pos = (int)str.find_first_of(delim, token_pos);
                
                if (delim_pos == std::string::npos)
                {
                    delim_pos = (int)str.size();
                }
            }

            len = delim_pos - token_pos;
            
            if (len)
            {
                vec.push_back(str.substr(token_pos, len));
            }
            else
            {
                vec.push_back("");
            }
            
            token_pos = delim_pos + 1;
            
        } while (delim_pos < (int)str.size());
    }
    

    inline int get_line(io_stream &is, std::string &line, int timeout = -1, int max_length = 512)
    {
        // max_length doesn't include crlf or a null terminator
        
        if (max_length < 0)
        {
            line.clear();
            return -1;
        }
        
        if (is.closed())
        {
            line.clear();
            return -1;
        }
        
        std::vector<char> buf(max_length + 3); // Add 2 for crlf, and 1 for a null terminator

        // end points to the last char in the buffer, reserved for a null terminator
        char *end = buf.data() + buf.size() - 1;
        char *ptr = buf.data();
        
        for (; ptr != end; ++ptr)
        {
            if (is.closed())
            {
                break;
            }
            
            int read_count = is.read(ptr, 1, timeout);

            if (read_count != 1)
            {
                break;
            }

            if (*ptr == '\r')
            {
                *ptr = 0;
            }
            else if (*ptr == '\n')
            {
                break;
            }
            else if (!*ptr)
            {
                break;
            }
        }

        *ptr = 0;
        
        line = buf.data();

        return (int)(end - buf.data());
    }


    inline int parse_header(const std::string &line, std::string &name, std::string &val)
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
        
        if(val_len)
            val = trim(line.substr(pos + 1, val_len));
        else
            val = "";
        
        return 0;
    }


    inline int read_all_headers(io_stream &is, std::map<std::string, std::string> &headers, int timeout = -1)
    {
        headers.clear();
        int rc = 0;
        int len = 0;
        int total = 0;
        std::string line;
        std::string name;
        std::string val;

        while (true)
        {
            len = get_line(is, line, timeout);

            if (len == 0)
            {
                break;
            }
            else if (len < 0)
            {
                headers.clear();
                return -1;
            }
            
            total += len;

            rc = parse_header(line, name, val);
            
            if (rc < 0)
            {
                headers.clear();
                return -1;
            }

            if (headers.count(name))
            {
                headers[name] += "," + val;
            }
            else
            {
                headers[name] = val;
            }
        }

        return total;
    }


    inline int read_all_headers(std::istream &is, std::map<std::string, std::string> &headers)
    {
        headers.clear();
        int rc = 0;
        int len = 0;
        int total = 0;
        std::string line;
        std::string name;
        std::string val;
        
        while (true)
        {
            std::getline(is, line);
            len = (int)is.gcount();
            chomp(line);
            
            if (len == 0)
            {
                break;
            }
            else if (len < 0)
            {
                headers.clear();
                return -1;
            }
            
            total += len;
            
            rc = parse_header(line, name, val);
            
            if (rc < 0)
            {
                headers.clear();
                return -1;
            }
            
            if (headers.count(name))
            {
                headers[name] += "," + val;
            }
            else
            {
                headers[name] = val;
            }
        }
        
        return total;
    }
    

    inline std::string get_path_end(const std::string &path, int count = 1)
    {
        if (!path.size())
        {
            return "";
        }

        std::vector<std::string> parts;
        split(path, "\\/", parts);

        int start = (int)parts.size() - count;

        if (start < 0)
        {
            start = 0;
        }

        std::string result;

        for (int i = start; i < (int)parts.size(); ++i)
        {
            if (i > start)
            {
                result += "/";
            }

            result += parts[i];
        }

        return result;
    }


    inline std::string get_filename(const std::string &path)
    {
        if (!path.size())
        {
            return "";
        }

        size_t slash_stop = path.find_last_of("\\/");

        if (slash_stop == path.size() - 1)
        {
            return "";
        }

        if (slash_stop == std::string::npos)
        {
            return path;
        }

        return path.substr(slash_stop + 1);
    }
    

    inline int find_positions(std::string &text, const std::string &find_string, std::vector<size_t> &pos)
    {
        pos.clear();
        
        if (!text.size())
        {
            return 0;
        }
        
        size_t start = 0;
        size_t f = 0;
        
        while (1)
        {
            f = text.find(find_string, start);
            
            if (f == std::string::npos)
            {
                return 0;
            }
            
            pos.push_back(f);
            
            start = f + find_string.size();
            
            if (start >= text.size())
            {
                return 0;
            }
        }
        
        return 0;
    }
    

    inline int replace_all(std::string &text, const std::map<std::string, std::string> &replace_map)
    {
        if (!text.size())
        {
            return -1;
        }
        
        if(!replace_map.size())
            return 0;
        
        std::map<size_t, int> pos_to_string;
        
        std::vector<std::map<std::string, std::string>::const_iterator> index_to_iterator;
        
        for (auto it = replace_map.begin(); it != replace_map.end(); ++it)
        {
            std::vector<size_t> pos;
            find_positions(text, it->first, pos);
            
            for (auto p : pos)
            {
                pos_to_string[p] = (int)index_to_iterator.size();
            }
            
            index_to_iterator.push_back(it);
        }
        
        size_t start = 0;
        size_t pos;
        size_t len;
        std::stringstream ss;
        
        for (auto pair1 : pos_to_string)
        {
            pos = pair1.first;
            len = pos - start;
            ss << text.substr(start, len);
            
            ss << index_to_iterator[pair1.second]->second;
            start = pos + index_to_iterator[pair1.second]->first.size();
        }
        
        ss << text.substr(start);
        text = ss.str();
        return 0;
    }
    

    inline std::string escape(const std::string &str)
    {
        std::map<std::string, std::string> m;
        m["<"] = "&lt;";
        m[">"] = "&gt;";
        m["&"] = "&amp;";
        m["'"] = "&apos;";
        m["\""] = "&quot;";
        std::string s = str;
        replace_all(s, m);
        return s;
    }
}

#endif


