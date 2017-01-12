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
#include <bravo/argv_parser.h>
#include <bravo/url.h>
#include <bravo/base_socket.h>
#include <bravo/ssl_socket.h>
#include <bravo/tls_socket.h>
#include <bravo/string_utils.h>
#include <bravo/hex.h>
#include <bravo/http_server.h>
#include <bravo/http_listen_port.h>

using namespace std;
using namespace bravo;

using namespace bravo;

class cmd_line_parser : public argv_parser
{
public:
    cmd_line_parser()
    {
        flag_defs["h"]          = AP_NO_ARG;
        flag_defs["help"]       = AP_NO_ARG;
        flag_defs["v"]          = AP_NO_ARG;
        flag_defs["version"]    = AP_NO_ARG;
        flag_defs["verbose"]    = AP_NO_ARG;
        
        usage_ = "usage:\nserrano";
    }
};

cmd_line_parser cmd_line;

int main(int argc, const char *argv[])
{
    cmd_line.parse(argc,argv);
    
    std::string home;

#ifdef _WIN32
    home = std::getenv("USERPROFILE");
#else
    home = std::getenv("HOME");
#endif

    set_server_cert_file(home + "/Desktop/projects/mustang/certs/servercert.pem");
    set_server_key_file(home + "/Desktop/projects/mustang/certs/serverkey.pem");
    set_client_ca_file(home + "/Desktop/projects/mustang/certs/ca-certificates.crt");

    http_server server;
    server.add_dir("", home + "/Desktop/projects/mustang/research/serrano/web", dir_specs::text);
    server.add_dir("cgi-bin", home + "/Desktop/projects/mustang/research/serrano/web/cgi-bin-win", dir_specs::exec);

    {
        server.add_port(new http_listen_port(2002, home + "/Desktop/projects/mustang/certs/servercert.pem", home + "/Desktop/projects/mustang/certs/serverkey.pem"));
        server.add_port(new http_listen_port(2001));
        server.start();

//        for (int i = 0; i < 10; ++i)
            for (;;)
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    server.stop();

    for (int i = 0; i < 10; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    server.start();

    for (;;)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}
