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

#ifdef _WIN32
    typedef __int64 ssize_t;
#endif

#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <bravo/http_listen_port.h>
#include <bravo/dir_utils.h>
#include <bravo/string_utils.h>
#include <bravo/http_utils.h>
#include <bravo/socket_stream.h>
#include <bravo/io_named_pipe.h>

using namespace std;

// Good reference:
//http://stackoverflow.com/questions/1547899/which-characters-make-a-url-invalid

namespace bravo
{ 
http_listen_port::http_listen_port(unsigned short port, bool secure, int timeout) :
    base_listen_port(port, secure, timeout)
{
    init();
}

http_listen_port::http_listen_port(unsigned short port, const std::string &cert_file, const std::string &key_file, int timeout) : base_listen_port(port, true, timeout)
{
    cert_file_      = cert_file;
    key_file_       = key_file;
    init();
}

void http_listen_port::init()
{
    keep_alive_max = 5;
#ifdef _WIN32
    os_name         = "Windows";
#else
    os_name         = "macOS";
#endif
    http_version    = "HTTP/1.1";
    server_name     = "DataServer";
    server_version  = "0.0.0";
    distro_name     = "";
}

void http_listen_port::add_dir(const std::string &dir, const std::string &actual_dir, dir_specs::dir_type type)
{
    dir_map[dir] = dir_specs(actual_dir, type);
}

int http_listen_port::process_connection(std::shared_ptr<socket_task> &task)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    socket_stream ss(task->sock_.get());

    while (!task->stop_)
    {
        http_message request;
        int rc = request.read_from(ss);
        
        if (rc <= 0)
            break;

        rc = process_http_request(ss, request);

        ss.flush();
        if (rc || task->sock_->closed())
            break;
    }

    ss.flush();
    return 0;
}

dir_specs::dir_type http_listen_port::dir_type(const string &dir)
{
    if (!dir_map.count(dir))
        return dir_specs::unknown;
    
    dir_specs specs = dir_map[dir];
    return specs.type;
}

int http_listen_port::build_response(http_message &request, http_message &response)
{
    response.type = dir_type(request.dir);
    
    if (response.type == dir_specs::unknown)
        return -1;
    
    response.sock = request.sock;
    string request_dir = "/";
    
    if (request.dir.size())
        request_dir += request.dir + "/";
    
    string real_dir = dir_map[request.dir].name + "/";
    string real_filename;
    
    if (request.filename == "")
        real_filename = "index.html";
    else
        real_filename = request.filename;
    
    std::string sub_path;
    
    if (request.sub_dir.size())
        sub_path = request.sub_dir + "/";

    sub_path += real_filename;
    string requested_path = request_dir + sub_path;
    response.full_path = real_dir + sub_path;
    vector<char> buf;
    bool file_found = (response.type != dir_specs::unknown) && file_exists(response.full_path);
    string status;
    string content_type_string;
    
    if (!file_found)
    {
        status = "404 Not Found";
        content_type_string = content_types.get_type("html");
        
        /* EXAMPLE
         HTTP/1.1 404 Not Found
         Date: Thu, 05 Nov 2015 16:43:24 GMT
         Server: Apache/2.0.58 (Win32) PHP/5.1.4
         Content-Length: 295
         Keep-Alive: timeout=15, max=99
         Connection: Keep-Alive
         Content-Type: text/html; charset=iso-8859-1
         */
        
        string not_found = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
        "<html><head>"
        "<title>404 Not Found</title>"
        "</head><body>"
        "<h1>Not Found</h1>"
        "<p>The requested URL " + requested_path + " was not found on this server.</p>"
        "<hr>"
        "<address>" + server_name + "/" + server_version + " (" + os_name + ") ";
        
        if (distro_name.size())
            not_found += "(" + distro_name + ") ";
        
        not_found += "Server at localhost Port " + std::to_string(port_) + "</address>"
                    "</body></html>";
        response.content = not_found;
    }
    else
    {
        status = "200 OK";
        
        if (response.type == dir_specs::exec)
            content_type_string = content_types.get_type("html"); // assume executable generates html
        else
            content_type_string = content_types.get_type(get_file_ext(real_filename));
        
        if (process_params(request))
            return -1;
    }
    
    response.request                        = false;
    response.http_version                   = http_version;
    response.status                         = status;
    response.headers["Cache-Control"]       = "no-cache";
    response.headers["Server"]              = server_name;
    response.headers["Content-Type"]        = content_type_string;
    response.headers["Transfer-Encoding"]   = "Chunked";
    
    if (keep_alive_max > 1)
    {
        string val = "timeout=";
        val += std::to_string(timeout_ / 1000) + ", max=" + std::to_string(keep_alive_max);
        response.headers["Keep-Alive"] = val;
        response.headers["Connection"] = "Keep-Alive";
    }
    else
        response.headers["Connection"] = "close";
    
    return 0;
}

int http_listen_port::process_http_request(std::ostream &os, http_message &request)
{
    http_message response;
    
    if (build_response(request, response))
        return -1;

    if (response.status == "404 Not Found")
    {
        if (response.write_to(os) < 0)
            return -1;
    }
    else
    {
        switch(response.type)
        {
            case dir_specs::exec:
                return process_http_exec_request(os, request, response);
                
            case dir_specs::text:
                return process_http_file_request(os, request, response);
                
            default:
            case dir_specs::unknown:
                break;
        }
    }
    
    return 0;
}

// Good reference:
// http://stackoverflow.com/questions/19598326/easiest-way-to-execute-linux-program-and-communicate-with-it-via-stdin-stdout-in

int http_listen_port::process_http_exec_request(std::ostream &os, http_message &request, http_message &response)
{
    // Write response headers.
    if (response.write_to(request.sock) < 0)
        return -1;
    
    const char *newargv[] = { NULL, "hello", "world", NULL };
    const char *newenviron[] = { NULL };

#ifdef _WIN32
    #define BUFSIZE 4096
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hInputFile = NULL;
    SECURITY_ATTRIBUTES saAttr;

    // Set the bInheritHandle flag so pipe handles are inherited.
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
    if (!bravo::CreatePipe2(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr))
        return -1;

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        return -1;

    // Create a pipe for the child process's STDIN.
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        return -1;

    // Ensure the write handle to the pipe for STDIN is not inherited.
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        return -1;

    // Create the child process. 
    TCHAR szCmdline[] = TEXT("child");
    wstring appname(response.full_path.begin(), response.full_path.end());
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process.
    HANDLE h1 = INVALID_HANDLE_VALUE;
    HANDLE h2 = INVALID_HANDLE_VALUE;
    bravo::CreatePipe2(&h1, &h2);
    bravo::io_named_pipe pipe2;
    pipe2.set_handle(h1);
    bSuccess = CreateProcess(appname.c_str(),
        szCmdline,     // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application.
    if (!bSuccess)
        return -1;
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example.
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }

    // Get a handle to an input file for the parent. 
    // This example assumes a plain text file and uses string output to verify data flow.
    DWORD dwRead = 0;
    CHAR chBuf[BUFSIZE];
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD actual = 0;
    stringstream ss;
    ss << "handles=" << (uint64_t)h2 << "\n";
    string params = ss.str();
    BOOL rc = WriteFile(g_hChildStd_IN_Wr, params.c_str(), (DWORD)params.size(), &actual, NULL);
    rc = FlushFileBuffers(g_hChildStd_IN_Wr);
    int errnum = GetLastError();
    io_named_pipe child_out;
    child_out.set_handle(g_hChildStd_OUT_Rd);
    
    if (g_hChildStd_OUT_Wr != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hChildStd_OUT_Wr);
        g_hChildStd_OUT_Wr = INVALID_HANDLE_VALUE;
    }

    for (;;)
    {
        dwRead = child_out.read(chBuf, BUFSIZE, 1000);
        
        if (child_out.handle == INVALID_HANDLE_VALUE)
            break;

        int total = 0;
        
        if (dwRead)
        {
            int len = write_chunk(request.sock, chBuf, dwRead);
            if (len < 0)
                return -6;

            total += len;
        }

        pipe2.write("hello", 7);
        pipe2.flush();
        dwRead = child_out.read(chBuf, BUFSIZE);

        if (child_out.handle == INVALID_HANDLE_VALUE)
            break;

        if (dwRead)
            int rc2 = write_chunk(request.sock, chBuf, dwRead);
    }

    int rc3 = write_chunk(request.sock, "");

#else
   
    int child_in[2];
    
    if(pipe(child_in) || child_in[0] == STDERR_FILENO || child_in[1] == STDERR_FILENO)
        return -1;
    
    int child_out[2];
    
    if(pipe(child_out) || child_out[0] == STDERR_FILENO || child_out[1] == STDERR_FILENO)
    {
        ::close(child_in[0]);
        ::close(child_in[1]);
        return -1;
    }
    
    int pid = fork();
    
    if (pid < 0)
    {
        ::close(child_in[0]);
        ::close(child_in[1]);
        ::close(child_out[0]);
        ::close(child_out[1]);
        return -1;
    }
    
    if (pid == 0)
    {
        // Child
        fflush(0);
    
        if (dup2(child_in[0], STDIN_FILENO) < 0 || dup2(child_out[1], STDOUT_FILENO) < 0)
            return -1;

        ::close(child_in[0]);
        ::close(child_in[1]);
        ::close(child_out[0]);
        ::close(child_out[1]);
        execve(response.full_path.c_str(), (char**)newargv, (char**)newenviron);
        return -1;
    }

    #define BUFFER_SIZE 512

    // Parent
    ::close(child_in[0]);
    ::close(child_out[1]);
    int child_input = child_in[1];
    int child_output = child_out[0];
    fcntl(child_input, F_SETFL, fcntl(child_input, F_GETFL) | O_NONBLOCK);
    fcntl(child_output, F_SETFL, fcntl(child_output, F_GETFL) | O_NONBLOCK);
    bool input_closed = false;
    bool output_closed = false;
    
    while(1)
    {
        bool input_would_block = false;
        bool output_would_block = false;
        char buf[BUFFER_SIZE];
        ssize_t rc1 = read(child_output, buf, BUFFER_SIZE);
        
        if (rc1 == 0)
        {
            input_closed = true;
            break;
        }
        else if (rc1 < 0 || rc1 > BUFFER_SIZE)
            input_would_block = true;
        else
        {
            write_chunk(request.sock, buf, (int32_t)rc1);
            write_chunk(request.sock, "");
        }
        
        int32_t stream_read_count = request.sock->read(buf, BUFFER_SIZE, 0);
        
        if (stream_read_count < 0)
        {
            if (request.sock->closed())
            {
                output_closed = true;
                break;
            }
        }
        else if (stream_read_count > 0)
            write(child_input, buf, stream_read_count);
        
        if (input_would_block && output_would_block)
            this_thread::sleep_for(chrono::milliseconds(1));
    }
    
#endif
    
    return 0;
}

int http_listen_port::write_content(std::ostream &os, http_message &request, http_message &response)
{
    cout << "write content\n";
    vector<char> buf;
    
    if(response.headers["Content-Type"] == "text/html")
    {
        string text;
        
        if (read_file(response.full_path, buf, true))
            return -1;
        
        text = buf.data();
        cout << "content: " << text << "\n";
        generate_content(request,text);
        
        if (write_chunk(os, text) != text.size())
            return -1;
        
        if (write_chunk(os, ""))
            return -1;
        
        os.flush();
    }
    else
    {
        if (read_file(response.full_path, buf))
            return -1;
        
        if (write_chunk(os, buf) < 0)
            return -1;
        
        if (write_chunk(os, "") < 0)
            return -1;
    }
    
    return 0;
}

int http_listen_port::process_http_file_request(std::ostream &os, http_message &request, http_message &response)
{
    if (response.write_to(os) < 0)
        return -1;

    os.flush();
    return write_content(os, request, response);
}

int http_listen_port::process_params(http_message &request)
{
    return 0;
}

int http_listen_port::generate_content(http_message &request, std::string &content)
{
    string prefix = "<%=Application(\"";
    string suffix = "\")%>";
    map<string,string> vars;
    
    for (auto &pair : request.app_vars)
        vars[prefix+pair.first+suffix] = pair.second;
    
    bravo::replace_all(content,vars);
    return 0;
}

void http_listen_port::generate_response_header(const std::string &ext, size_t content_length, std::string &response_header)
{
    stringstream ss;
    ss << "HTTP/1.0 200 OK\r\n";
    ss << "Cache-Control: no-cache\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-Length: " << content_length << "\r\n";
    ss << "Content-Type: " << content_types.get_type(ext);
    ss << "\r\n";
    response_header = ss.str();
}

}

