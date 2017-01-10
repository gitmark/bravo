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

#ifndef __http_listen_port_h__
#define __http_listen_port_h__

#include <bravo/base_listen_port.h>
#include <bravo/http_message.h>
#include <bravo/dir_specs.h>
#include <bravo/content_types.h>

namespace bravo
{
class http_listen_port : public base_listen_port
{
public:
    http_listen_port    (unsigned short port, bool secure = false, int timeout = base_socket::default_timeout_);
    http_listen_port    (unsigned short port, const std::string &cert_file, const std::string &key_file,
                         int timeout = base_socket::default_timeout_);

    virtual int process_connection          (std::shared_ptr<socket_task> &task);
    virtual int process_params              (http_message &request);
    virtual int process_http_request        (http_message &request);
    virtual int process_http_file_request   (http_message &request, http_message &response);
    virtual int process_http_exec_request   (http_message &request, http_message &response);
    void        generate_response_header    (const std::string &ext, size_t content_length, std::string &response_header);
    virtual int generate_content            (http_message &request, std::string &content);
    virtual int build_response              (http_message &request, http_message &response);
    virtual int write_content               (http_message &request, http_message &response);
    void        add_dir                     (const std::string &dir, const std::string &actual_dir,
                                             dir_specs::dir_type type);
    void        init                        ();
    dir_specs::dir_type dir_type            (const std::string &dir);
    
    int             keep_alive_max;
    std::string     http_version;
    std::string     server_name;
    std::string     server_version;
    std::string     os_name;
    std::string     distro_name;
    content_types_t content_types;
    std::map<std::string, dir_specs> dir_map;
};
    
}

#endif
