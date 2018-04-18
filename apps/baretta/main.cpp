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

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <bravo/argv_parser.h>
#include <bravo/url.h>
#include <bravo/base_socket.h>
#include <bravo/ssl_socket.h>
#include <bravo/tls_socket.h>
#include <bravo/string_utils.h>
#include <bravo/hex.h>


using namespace std;
using namespace bravo;

int hex_to_num1[] = {
//  0           4           8 9 A B     C D E F
   -1,0,0,0,    0,0,0,0,  -1,-1,-1,-1, -1,-1,0,0, // 0
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 1
    -1,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 2
    0,1,2,3,    4,5,6,7,    8,9,0,0,    0,0,0,0, // 3
    
    0,10,11,12, 13,14,15,0, 0,0,0,0,    0,0,0,0, // 4
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 5
    0,10,11,12, 13,14,15,0, 0,0,0,0,    0,0,0,0, // 6
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0, // 7
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
    0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0
};

char num_to_hex1[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


unsigned long long from_hex(const std::string str)
{
    if (!str.size())
    {
        return 0;
    }
    
    unsigned long long num = 0;
    
    int i = 0;
    
    while (hex_to_num1[(int)str[i]] == -1)
        ++i;
    
    int n;
    while ((n = hex_to_num1[(int)str[i++]]) >= 0)
    {
        num <<= 4;
        num += n;
    }
    
    return num;
}

std::string to_hex(unsigned long long num)
{
    char result[10];
    int r = 0;
    
    char *buf = (char*)&num;
    int count = sizeof(num);
    int i = count - 1;
    for (; i >= 0; --i)
    {
        if (buf[i])
            break;
    }
    
    if (i < 0)
        return "0";
    
    char b = buf[i];
    unsigned msb = (0xF0 & b) >> 4;
    
    if (msb)
        result[r++] = num_to_hex1[msb];
    
    unsigned lsb = (0x0F & b);
    result[r++] = num_to_hex1[lsb];
    --i;
    
    for (; i >= 0; --i)
    {
        b = buf[i];
        msb = (0xF0 & b) >> 4;
        result[r++] = num_to_hex1[msb];
        lsb = (0x0F & b);
        result[r++] = num_to_hex1[lsb];
    }
    
    result[r] = 0;
    return result;
}


std::string build_client_header(const std::string& host, const std::string& path)
{
    std::stringstream ss;
    ss << "GET " << path << " HTTP/1.1\r\n"
    "Host: " << host << "\r\n"
    "Connection: keep-alive\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML,like Gecko) Chrome/54.0.2840.99 Safari/537.36\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "Accept-Language: en-US,en;q=0.8\r\n"
    "\r\n";
    
    return ss.str();
}

std::string download(const std::string& url)
{
    std::string s_url = url;
    int code = 0;
    std::string content;
    while(1)
    {
        class url u;
        u.parse(s_url);
        if (!u.path.size())
            u.path = "/";
            
        std::map<std::string,std::string> m;
        
        if (u.valid)
        {
            base_socket *sock;
            
            if (u.secure())
            {
#ifdef _WIN32
                sock = new ssl_socket();
#else
                sock = new tls_socket();
#endif
            }
            else
            {
                sock = new norm_socket();
            }
            
            sock->connect(u.host, u.port_num());
            std::string client_header = build_client_header(u.host, u.path);
            cout << client_header << "\n";
            sock->write(client_header);
//            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            std::string str;
            std::string first_line;
            sock->read_line(first_line);
            vector<string> vec;
            bravo::split(first_line,' ',vec);
            
            if (vec.size() > 1)
                code = std::atoi(vec[1].c_str());
            else
                code = 0;
            
            std::string name;
            std::string val;
            
            while(sock->read_line(str) > 2)
            {
                cout << "line: " << str << "\n";
                parse_header(str,name,val);
                if(m.find(name) != m.end())
                {
                    m[name] += "," + val;
                }
                else
                {
                    m[name] = val;
                }
            }
            
            if((code == 302 || code == 301) && m.count("Location"))
            {
                s_url = m["Location"];
                continue;
            }
            
            string trans_encoding;
            if (m.count("Transfer-Encoding"))
            {
                trans_encoding = m["Transfer-Encoding"];
                std::transform(trans_encoding.begin(), trans_encoding.end(), trans_encoding.begin(), ::tolower);
            }
            
            if(trans_encoding == "chunked")
            {
                stringstream ss;
                int len = 0;
                do
                {
                    std::string s_len;
                    if (sock->read_line(s_len) < 2)
                        break;
                    
                    chomp(s_len);
                    len = (int)from_hex(s_len);
                    if (len < 1)
                        break;
                    
                    std::string chunk;
                    vector<char> vec(len + 1);
                    int r = sock->read(vec.data(), len);
                    if (r < 1)
                        break;
                    
                    vec[r] = 0;
                    ss << vec.data();
                    
                    if (sock->read_line(s_len) < 2)
                        break;
                    
                } while (len);
                
                content = ss.str();
            }
            else if(m.count("Content-Length"))
            {
                string snum = m["Content-Length"];
                int len = std::atoi(snum.c_str());
                vector<char> data(len+1);
                int r = sock->read(data.data(), len);
                
                if (r > 0)
                {
                    data[r] = 0;
                    content = data.data();
                }
            }
            else
            {
                cout << "error\n";
            }
        }
        break;
    }
    
    return content;
}



class cmd_line_parser : public argv_parser
{
public:
    cmd_line_parser()
    {
        flag_defs["o"]          = AP_REQUIRED_ARG;
        flag_defs["output"]     = AP_REQUIRED_ARG;
        flag_defs["h"]          = AP_NO_ARG;
        flag_defs["help"]       = AP_NO_ARG;
        flag_defs["v"]          = AP_NO_ARG;
        flag_defs["version"]    = AP_NO_ARG;
        flag_defs["verbose"]    = AP_NO_ARG;
        
        usage_ = "usage:\nbaretta url";
    }
};

cmd_line_parser cmd_line;


int main(int argc, const char *argv[])
{
    cmd_line.parse(argc,argv);
    
    std::string version = "0.0.0";
    
    if (cmd_line.flags.count("version"))
    {
        cout << "Baretta version " << version << "\n";
        return 0;
    }
    
    if (!cmd_line.args.size())
    {
        cout << cmd_line.error_msg();
        return 1;
    }


    
    std::string home;
    
#ifdef _WIN32
    home = std::getenv("USERPROFILE");
#else
    home = std::getenv("HOME");
#endif

//    set_server_cert_file(home + "/mustang/b-0.0.0/certs/serrano.cert.signed.pem");
//    set_server_key_file(home + "/mustang/b-0.0.0/certs/serrano.private.unsecured.key");
    set_client_ca_file(home + "/mustang/0.0.0/certs/rootCA.pem");
//   set_client_ca_file(home + "/mustang/0.0.0/certs/ca-certificates.crt");

    
    /*
    set_server_cert_file(home + "/mustang/b-0.0.0/certs/mycert.pem");
    set_server_key_file(home + "/mustang/b-0.0.0/certs/mykey.pem");
    set_client_ca_file(home + "/mustang/b-0.0.0/certs/mycert.pem");
    */
    std::string content = download(cmd_line.args[0]);
    cout << content << "\n";
    cout.flush();
    return 0;
}







