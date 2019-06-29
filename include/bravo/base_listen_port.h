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

#ifndef BASE_LISTEN_PORT_H_
#define BASE_LISTEN_PORT_H_

#ifdef _WIN32
    #define NOMINMAX
    #include <winsock2.h>
    #define socklen_t int
#else
    #define INVALID_SOCKET -1
#include <sys/ioctl.h>
#define INVALID_SOCKET -1
#define WSAGetLastError() errno
#define SOCKET_ERROR -1
#define ioctlsocket ioctl
#define WSAEWOULDBLOCK EWOULDBLOCK
#include <netinet/tcp.h>
#include <poll.h>
#endif

#include <condition_variable>
#include <thread>
#include <set>

#include <bravo/string_utils.h>
#include <bravo/tls_socket.h>
#include <bravo/ssl_socket.h>
#include <bravo/socket_utils.h>

namespace bravo
{
#define secure_socket ssl_socket
#define BACKLOG 5

class socket_task
{
public:
    socket_task() :
    stop_(true),
    running_(false),
    socket_(INVALID_SOCKET),
    secure_(false)
    {}
    
    bool                            stop_;
    bool                            running_;
    SOCKET                          socket_;
    bool                            secure_;
    std::unique_ptr<base_socket>    sock_;
    std::unique_ptr<std::thread>    thread_;
    std::mutex                      stop_mutex_;
    std::condition_variable         stop_cond_;
    std::string                     cert_file_;
    std::string                     key_file_;
};

class base_listen_port
{
public:
    base_listen_port(unsigned short port, bool secure, int timeout = -1) :
        port_(port),
        stop_(true),
        running_(false),
        timeout_(timeout),
        secure_(secure)
    {}

    virtual ~base_listen_port()
    {
        stop();
    }

    virtual int process_connection(std::shared_ptr<socket_task> &task)
    {
        return 0;
    }
    
    int call_process_connection(std::shared_ptr<socket_task> task)
    {
        if (secure_)
            task->sock_ = std::make_unique<secure_socket>(task->socket_, task->cert_file_, task->key_file_);
        else
            task->sock_ = std::make_unique<norm_socket>(task->socket_);
        
        int r = process_connection(task);
        
        if (!remove_socket_task(task))
        {
            task->stop_ = true;
            task->thread_->detach();
            task->thread_ = nullptr;
        }
        
        task->running_ = false;
        task->stop_cond_.notify_all();
        return r;
    }
    
    virtual int listen()
    {
        if (listen_sock_)
            return 0;
        
        if (secure_)
        {
            listen_sock_ = std::make_unique<secure_socket>();
            return listen_sock_->listen(port_);
        }
        else
        {
            listen_sock_ = std::make_unique<norm_socket>();
            return listen_sock_->listen(port_);
        }
    }
    
    std::unique_ptr<base_socket> accept(int timeout)
    {
        if (listen() < 0)
            return nullptr;
        
        std::unique_ptr<base_socket> ptr;
        int c = 0;
        
        while (c < timeout || timeout < 0)
        {
            SOCKET sock1 = listen_sock_->accept(1);
            
            if (sock1 == INVALID_SOCKET)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ++c;
                continue;
            }
            
            if (secure_)
                ptr = std::make_unique<secure_socket>(sock1, cert_file_, key_file_);
            else
                ptr = std::make_unique<norm_socket>(sock1);
            
            break;
        }

        return ptr;
    }
    
    virtual int run()
    {
        if (listen() < 0)
            return -1;
        
        while (!stop_)
        {
            SOCKET sock_ = listen_sock_->accept(10);
            
            
            if (sock_  == INVALID_SOCKET)
            {
                if (stop_)
                {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            int on = 1;
            int rc = setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));
            
            if (SOCKET_ERROR == rc)
            {
                safe_close(sock_);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            
            std::shared_ptr<socket_task> task = std::make_unique<socket_task>();
            task->socket_       = sock_;
            task->stop_         = false;
            task->running_      = true;
            task->cert_file_    = cert_file_;
            task->key_file_     = key_file_;
            add_socket_task(task);
            task->thread_       = std::make_unique<std::thread>(&base_listen_port::call_process_connection,
                                                                          this, task);
        }

        running_ = false;
        stop_cond_.notify_all();
        return 0;
    }

    virtual void start()
    {
        if (running_)
            return;

        running_ = true;
        stop_ = false;
        listen_thread_ = std::make_shared<std::thread>(&base_listen_port::run, this);
    }
    
    virtual void stop()
    {
        stop_ = true;
        
        {
            std::unique_lock<std::mutex> mlock(stop_mutex_);
            stop_cond_.wait(mlock, [=] { return !running_; });
        }
        
        if (listen_thread_ && listen_thread_->joinable())
            listen_thread_->join();

        listen_thread_ = nullptr;
        
        std::set<std::shared_ptr<socket_task>> tasks;
        
        {
            std::unique_lock<std::recursive_mutex> lock(socket_tasks_mutex_);
            
            for(auto task : socket_tasks_)
                task->stop_ = true;
            
            tasks = socket_tasks_;
        }
        
        for(auto task : tasks)
        {
            if (!remove_socket_task(task))
            {
                {
                    std::unique_lock<std::mutex> mlock(task->stop_mutex_);
                    task->stop_cond_.wait(mlock, [=] { return !task->running_; });
                }
                
                task->thread_->join();
            }
            
            task->thread_ = nullptr;
        }
        
        socket_tasks_.clear();
    }

    void add_socket_task(std::shared_ptr<socket_task> &task)
    {
        std::unique_lock<std::recursive_mutex> lock(socket_tasks_mutex_);
        socket_tasks_.insert(task);
    }

    int remove_socket_task(std::shared_ptr<socket_task> &task)
    {
        std::unique_lock<std::recursive_mutex> lock(socket_tasks_mutex_);
        
        if (socket_tasks_.count(task))
        {
            socket_tasks_.erase(task);
            return 0;
        }
        
        return -1;
    }
    
    void set_stop()
    {
        stop_ = true;
    }
    
    void lock()
    {
        socket_tasks_mutex_.lock();
    }
    
    void unlock()
    {
        socket_tasks_mutex_.unlock();
    }
    
    int close()
    {
        int rc = listen_sock_->close();
        listen_sock_ = nullptr;
        return rc;
    }
    
protected:
     unsigned short                          port_;
     bool                                    stop_;
     bool                                    running_;
     int                                     timeout_;
     bool                                    secure_;
     std::shared_ptr<std::thread>            listen_thread_;
     std::set<std::shared_ptr<socket_task>>  socket_tasks_;
     std::recursive_mutex                    socket_tasks_mutex_;
     std::mutex                              stop_mutex_;
     std::condition_variable                 stop_cond_;
     std::string                             base_dir_;
     sockaddr_in                             new_sock_addr_;
     std::unique_ptr<base_socket>            listen_sock_;
     std::string                             cert_file_;
     std::string                             key_file_;
};
    
}

#endif


