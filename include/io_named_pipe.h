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

#ifndef IO_NAMED_PIPE_H_
#define IO_NAMED_PIPE_H_

#ifdef _WIN32
#include <windows.h> 

#include <string>
#include <vector>

#include <bravo/io_stream.h>

#define OVERLAPPED_BUFSIZE 4096

// Good references:
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365603(v=vs.85).aspx


namespace bravo
{
    // overlapped_data encapsulates the OVERLAPPED structure. Three instances of overlapped_data
    // will be used with a single io_named_pipe instance. One for read, one for write and one
    // for connect.
    
    class overlapped_data
    {
    public:
        overlapped_data()
        {
            memset(&overlapped, 0, sizeof(overlapped));
            
            // Create event
            overlapped.hEvent           = CreateEvent(0, TRUE, TRUE, 0);
            current                     = buf;
            requested_transfer_count    = 0;
            actual_transfer_count       = 0;
            remaining_count             = 0;
            pending                     = false;
            timed_out                   = false;
        }

        virtual ~overlapped_data()
        {
            CloseHandle(overlapped.hEvent);
        }

        OVERLAPPED  overlapped;
        char        buf[OVERLAPPED_BUFSIZE];
        char *      current;
        DWORD       requested_transfer_count;
        DWORD       actual_transfer_count;
        int         remaining_count;
        bool        pending;
        bool        timed_out;
    };


    class io_named_pipe : bravo::io_stream
    {
    public:
        
        io_named_pipe(HANDLE h = INVALID_HANDLE_VALUE)
        {
            handle = h;
        }

        
        void set_handle(HANDLE h = INVALID_HANDLE_VALUE)
        {
            handle = h;
        }

        
        void detach_handle()
        {
            if (INVALID_HANDLE_VALUE == handle)
            {
                return;
            }

            cancel_pending_io();
            handle = INVALID_HANDLE_VALUE;
        }

        
        int listen()
        {
            // Connect named pipe
            BOOL connect_result = ConnectNamedPipe(handle, &connect_info.overlapped);

            if (connect_result)
            {
                // Successful connection
                connect_info.pending = false;
                SetEvent(connect_info.overlapped.hEvent);
            }
            else
            {
                int errnum = GetLastError();

                if (ERROR_IO_PENDING == errnum || ERROR_PIPE_LISTENING == errnum)
                {
                    connect_info.pending = true;
                }
                else if (ERROR_PIPE_CONNECTED == errnum)
                {
                    // Already connected
                    connect_info.pending = false;
                }
                else
                {
                    CloseHandle(handle);
                    handle = INVALID_HANDLE_VALUE;
                }
            }
            
            return 0;
        }

        
        int listen(const std::string &pipe_name, int instances = 1)
        {
            SECURITY_ATTRIBUTES saAttr;
            saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
            saAttr.bInheritHandle       = TRUE;
            saAttr.lpSecurityDescriptor = NULL;

            std::wstring wname(pipe_name.begin(), pipe_name.end());
            
            // Create named pipe
            handle = CreateNamedPipe(
                wname.c_str(),           
                PIPE_ACCESS_DUPLEX |     
                FILE_FLAG_OVERLAPPED,    
                PIPE_TYPE_BYTE |        // Not using a message type pipe.
                PIPE_READMODE_BYTE |     
                PIPE_WAIT,               
                instances,
                OVERLAPPED_BUFSIZE,
                OVERLAPPED_BUFSIZE,
                5000,                   // Client timeout
                &saAttr);

            if (INVALID_HANDLE_VALUE == handle)
                return -1;
            
            return listen();
        }

        // Client side method        
        int connect(const std::string &pipe_name, int timeout = -1)
        {        
            std::wstring wname(pipe_name.begin(), pipe_name.end());
            
            for (int i = 0; i < 2; i++)
            {
                handle = CreateFile(
                    wname.c_str(),  
                    GENERIC_READ |  
                    GENERIC_WRITE,
                    0,              
                    &saAttr,        
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED,
                    NULL);

                if (handle != INVALID_HANDLE_VALUE)
                    break;

                if (GetLastError() != ERROR_PIPE_BUSY)
                    return -1;

                if (timeout == 0)
                    return -1;

                if (timeout < 0)
                    timeout = NMPWAIT_WAIT_FOREVER;

                if (!WaitNamedPipe(wname.c_str(), timeout))
                    return -1;
            }

            if (handle == INVALID_HANDLE_VALUE)
                return -1;

            DWORD dwMode = PIPE_READMODE_BYTE;
            BOOL result = SetNamedPipeHandleState(
                handle,     
                &dwMode,    
                NULL,       
                NULL);

            if (!result)
            {
                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
                return -1;
            }

            return 0;
        }

        
        void cancel_pending_io()
        {
            if (INVALID_HANDLE_VALUE == handle)
            {
                return;
            }

            if (connect_info.pending)
            {
                BOOL cancel_result = CancelIoEx(handle, &connect_info.overlapped);

                if (cancel_result == TRUE || GetLastError() != ERROR_NOT_FOUND)
                {
                    DWORD transfer_count = 0;
                    GetOverlappedResult(handle, &connect_info.overlapped, &transfer_count, TRUE);
                }

                connect_info.pending = false;
            }

            if (read_info.pending)
            {
                BOOL result = CancelIoEx(handle, &read_info.overlapped);

                if (result == TRUE || GetLastError() != ERROR_NOT_FOUND)
                {
                    DWORD transfer_count = 0;
                    GetOverlappedResult(handle, &read_info.overlapped, &transfer_count, TRUE);
                }

                read_info.pending = false;
            }

            if (write_info.pending)
            {
                BOOL result = CancelIoEx(handle, &write_info.overlapped);

                if (result == TRUE || GetLastError() != ERROR_NOT_FOUND)
                {
                    DWORD transfer_count = 0;
                    GetOverlappedResult(handle, &write_info.overlapped, &transfer_count, TRUE);
                }

                write_info.pending = false;
            }
        }

        
        int close()
        {
            cancel_pending_io();
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
            return 0;
        }

        
        virtual ~io_named_pipe()
        {
            close();
        }

        
        int flush()
        {
            if (INVALID_HANDLE_VALUE == handle)
            {
                return -1;
            }

            FlushFileBuffers(handle);
            return 0;
        }

        
        int read(char *read_buf, int requested_count, int timeout = -1)
        {
            // Check internal state
            if (INVALID_HANDLE_VALUE == handle)
                return -1;
            
            // Check parameters
            if (!read_buf || requested_count < 0)
                return -1;
            
            if (requested_count == 0)
                return 0; // Odd, but not an error
            
            // Init local vars
            char *buf       = read_buf;
            int remaining   = requested_count;
            int attempted   = 0;
            
            // First check to see if we are connected yet.
            if (connect_info.pending)
            {
                // If timeout == 0 the caller doesn't want to wait, thus return now because the connection is pending.
                if (timeout == 0)
                {
                    // Timed out, not an error
                    return 0;
                }
                
                // If we reach this point, we know that timeout is non-zero
                if (timeout < 0)
                    timeout = INFINITE;

                DWORD wait_result = WaitForSingleObject(connect_info.overlapped.hEvent, timeout);

                if (WAIT_OBJECT_0 == wait_result)
                {
                    // Don't return, fall through and call GetOverlappedResult()
                }
                else if (WAIT_TIMEOUT == wait_result)
                {
                    // Timed out, not an error
                    return 0;
                }
                else
                {
                    // Error condition
                    close();
                    return -1;
                }
                    
                // If we reach this point we know that WaitForSingleObject() was successful
                // and we are ready to call GetOverlappedResult()
                DWORD transfer_count = 0;
                
                BOOL overlapped_result = GetOverlappedResult(
                    handle,
                    &connect_info.overlapped,
                    &transfer_count,
                    FALSE);

                if (overlapped_result)
                {
                    // Successful connect
                    connect_info.pending = false;
                    
                    // Fall through and call ReadFile()
                }
                else
                {
                    int errnum = GetLastError();

                    if (ERROR_IO_PENDING == errnum || ERROR_PIPE_LISTENING == errnum || ERROR_IO_INCOMPLETE == errnum)
                    {
                        // Timed out, not an error
                        return 0;
                    }
                    else
                    {
                        // Error condition
                        close();
                        return -1;
                    }
                }
            }

            // If we reach this point we know we are connected.
            // If read_info.remaining_count > 0 we have data cached from the last read and we should
            // get this data before we even attempt ReadFile()
            
            if (read_info.remaining_count > 0)
            {
                attempted = std::min(remaining, read_info.remaining_count);
 
                memcpy(buf, read_info.current, attempted);
                read_info.remaining_count -= attempted;
                read_info.current += attempted;
                remaining -= attempted;
                buf += attempted;
                
                if (!remaining)
                    return buf - read_buf;
            }

            // The only reasons we will exit from the loop below are
            // 1) An error condition
            // 2) We read the full count the caller requested, remaining == 0
            // 3) We timed out
            // 4) We read 0 bytes with no error
        
            while(1)
            {
                // If we reach this point, we know we still need to read data.
                if (!read_info.pending)
                {
                    // At this point we know that read_info is empty so we are free to set it up.
                    
                    attempted = std::min(remaining, OVERLAPPED_BUFSIZE);
                    read_info.requested_transfer_count = attempted;
                    read_info.actual_transfer_count = 0;
                    read_info.current = read_info.buf;
                    read_info.remaining_count = 0;
                    
                    BOOL read_result = ReadFile(handle,
                        read_info.buf,
                        read_info.requested_transfer_count,
                        &read_info.actual_transfer_count,
                        &read_info.overlapped);

                    if (read_result)
                    {
                        if (read_info.actual_transfer_count)
                        {
                            read_info.remaining_count = read_info.actual_transfer_count;
                            attempted = std::min(remaining, read_info.remaining_count);
                            
                            memcpy(buf, read_info.current, attempted);
                            read_info.remaining_count -= attempted;
                            read_info.current += attempted;
                            remaining -= attempted;
                            buf += attempted;

                            if (!remaining)
                            {
                                // Remaining == 0, we've read everything the caller asked for so return now
                                return buf - read_buf;
                            }
                            else
                            {
                                // We still have more to read and overlapped io is not pending so we
                                // continue with the loop to call ReadFile() again.
                                continue;
                            }
                        }
                        else
                        {
                            // We read 0 count without an error, so just return what's been read so far
                            return buf - read_buf;
                        }
                    }
                    else
                    {
                        int errnum = GetLastError();

                        if (ERROR_IO_PENDING == errnum)
                        {
                            read_info.pending = true;
                            // Fall through and call WaitForSingleObject()
                        }
                        else
                        {
                            // Error condition
                            close();
                            return -1;
                        }
                    }
                }
                    
                // If we reach this point we know that data is pending
                // If timeout == 0, the caller doesn't want to wait, thus return now because data is pending.
                if (timeout == 0)
                {
                    // Timed out, not an error
                    return buf - read_buf;
                }
                
                if (timeout < 0)
                {
                    timeout = INFINITE;
                }

                DWORD wait_result = WaitForSingleObject(read_info.overlapped.hEvent, timeout);

                if (WAIT_OBJECT_0 == wait_result)
                {
                    read_info.actual_transfer_count = 0;
                    read_info.current = read_info.buf;
                    read_info.remaining_count = 0;
                    
                    BOOL overlapped_result = GetOverlappedResult(
                        handle,
                        &read_info.overlapped,
                        &read_info.actual_transfer_count,
                        FALSE);

                    if (overlapped_result)
                    {
                        read_info.pending = false;
                        
                        if (read_info.actual_transfer_count)
                        {
                            read_info.remaining_count = read_info.actual_transfer_count;
                            attempted = std::min(remaining, read_info.remaining_count);
                            
                            memcpy(buf, read_info.current, attempted);
                            read_info.remaining_count -= attempted;
                            read_info.current += attempted;
                            remaining -= attempted;
                            buf += attempted;
                            
                            if (!remaining)
                            {
                                // Remaining == 0, we've read everything the caller asked for so return now
                                return buf - read_buf;
                            }
                            else
                            {
                                // We still have more to read and overlapped io is not pending so we
                                // continue with the loop to call ReadFile() again.
                                continue;
                            }
                        }
                        else
                        {
                            // We read 0 count without an error, so just return what's been read so far
                            return buf - read_buf;
                        }
                    }
                    else
                    {
                        // Error condition
                        close();
                        return -1;
                    }
                }
                else if (WAIT_TIMEOUT == wait_result)
                {
                    // Timed out
                    return buf - read_buf;
                }
                else
                {
                    // Error condition
                    close();
                    return -1;
                }
            } // while(1)

            // Should never actually reach this point
            return 0;
        }

        
        int write(const char *write_buf, int requested_count, int timeout = -1)
        {
            // Check internal state
            if (INVALID_HANDLE_VALUE == handle)
                return -1;
            
            // Check parameters
            if (!write_buf || requested_count < 0)
                return -1;
            
            if (requested_count == 0)
                return 0; // Odd, but not an error
            
            // Init local vars
            char *buf       = write_buf;
            int remaining   = requested_count;
            int attempted   = 0;
            
            // First check to see if we are connected yet.
            if (connect_info.pending)
            {
                // If timeout == 0 the caller doesn't want to wait, thus return now because the connection is pending.
                if (timeout == 0)
                {
                    // Timed out, not an error
                    return 0;
                }
                
                // If we reach this point, we know that timeout is non-zero
                if (timeout < 0)
                    timeout = INFINITE;
                
                DWORD wait_result = WaitForSingleObject(connect_info.overlapped.hEvent, timeout);
                
                if (WAIT_OBJECT_0 == wait_result)
                {
                    // Don't return, fall through and call GetOverlappedResult()
                }
                else if (WAIT_TIMEOUT == wait_result)
                {
                    // Timed out, not an error
                    return 0;
                }
                else
                {
                    // Error condition
                    close();
                    return -1;
                }
                
                // If we reach this point we know that WaitForSingleObject() was successful
                // and we are ready to call GetOverlappedResult()
                DWORD transfer_count = 0;
                
                BOOL overlapped_result = GetOverlappedResult(
                                                             handle,
                                                             &connect_info.overlapped,
                                                             &transfer_count,
                                                             FALSE);
                
                if (overlapped_result)
                {
                    // Successful connect
                    connect_info.pending = false;
                    
                    // Fall through and call WriteFile()
                }
                else
                {
                    int errnum = GetLastError();
                    
                    if (ERROR_IO_PENDING == errnum || ERROR_PIPE_LISTENING == errnum || ERROR_IO_INCOMPLETE == errnum)
                    {
                        // Timed out, not an error
                        return 0;
                    }
                    else
                    {
                        // Error condition
                        close();
                        return -1;
                    }
                }
            }
            
            // The only reasons we will exit from the loop below are
            // 1) An error condition
            // 2) We wrote the full count the caller requested, remaining == 0 and write_info.requested_count == 0
            // 3) We timed out
            // 4) We wrote 0 bytes with no error
            
            while(1)
            {
                // If we reach this point, we know we still need to write data.
                
                if (!write_info.pending)
                {
                    // Because write data is not pending, we know that write_info.remaining_count is valid.
 
                    // If there is space remaining at the end of the buffer ...
                    if (write_info.remaining_count > 0)
                    {
                        attempted = std::min(remaining, write_info.remaining_count);
                        
                        memcpy(write_info.current, buf, attempted);
                        write_info.remaining_count -= attempted;
                        write_info.current += attempted;
                        remaining -= attempted;
                        buf += attempted;
                    }
                    
                    write_info.requested_transfer_count = write_info.current - write_info.buf;
                    write_info.actual_transfer_count = 0;
                    
                    BOOL write_result = WriteFile(handle,
                                                write_info.buf,
                                                write_info.requested_transfer_count,
                                                &write_info.actual_transfer_count,
                                                &write_info.overlapped);
                    
                    if (write_result)
                    {
                        int delta = write_info.requested_transfer_count - write_info.actual_transfer_count;
                        
                        if (delta)
                            std::memmove(write_info.buf, write_info.buf + write_info.actual_transfer_count, delta);
                        
                        write_info.current = write_info.buf + delta;
                        write_info.remaining_count = OVERLAPPED_BUFSIZE - delta;
                        write_info.requested_transfer_count = delta;
                        
                        if (write_info.actual_transfer_count)
                        {
                            if (!write_info.requested_transfer_count && !remaining)
                            {
                                // We've written everything the caller asked for so return now
                                return buf - write_buf;
                            }
                            else
                            {
                                // We still have more to write and overlapped io is not pending so we
                                // continue with the loop to call WriteFile() again.
                                continue;
                            }
                        }
                        else
                        {
                            // We wrote 0 count without an error, so just return what's been written so far
                            return buf - write_buf;
                        }
                    }
                    else
                    {
                        int errnum = GetLastError();
                        
                        if (ERROR_IO_PENDING == errnum)
                        {
                            write_info.pending = true;
                            // Fall through and call WaitForSingleObject()
                        }
                        else
                        {
                            // Error condition
                            close();
                            return -1;
                        }
                    }
                }
                
                // If we reach this point we know that data is pending
                // If timeout == 0, the caller doesn't want to wait, thus return now because data is pending.
                if (timeout == 0)
                {
                    // Timed out, not an error
                    return buf - write_buf;
                }
                
                if (timeout < 0)
                {
                    timeout = INFINITE;
                }
                
                DWORD wait_result = WaitForSingleObject(write_info.overlapped.hEvent, timeout);
                
                if (WAIT_OBJECT_0 == wait_result)
                {
                    write_info.actual_transfer_count = 0;

                    BOOL overlapped_result = GetOverlappedResult(
                                                                 handle,
                                                                 &write_info.overlapped,
                                                                 &write_info.actual_transfer_count,
                                                                 FALSE);
                    
                    if (overlapped_result)
                    {
                        write_info.pending = false;
                        
                        int delta = write_info.requested_transfer_count - write_info.actual_transfer_count;
                        
                        if (delta)
                            std::memmove(write_info.buf, write_info.buf + write_info.actual_transfer_count, delta);
                        
                        write_info.current = write_info.buf + delta;
                        write_info.remaining_count = OVERLAPPED_BUFSIZE - delta;
                        write_info.requested_transfer_count = delta;
                        
                        if (write_info.actual_transfer_count)
                        {
                            if (!write_info.requested_transfer_count && !remaining)
                            {
                                // We've written everything the caller asked for so return now.
                                return buf - write_buf;
                            }
                            else
                            {
                                // We still have more to write and overlapped io is not pending so we
                                // continue with the loop to call WriteFile() again.
                                continue;
                            }
                        }
                        else
                        {
                            // We wrote 0 bytes without an error, so just return what's been written so far
                            return buf - write_buf;
                        }
                    }
                    else
                    {
                        // Error condition
                        close();
                        return -1;
                    }
                }
                else if (WAIT_TIMEOUT == wait_result)
                {
                    // Timed out
                    return buf - write_buf;
                }
                else
                {
                    // Error condition
                    close();
                    return -1;
                }
            } // while(1)
            
            // Should never actually reach this point
            return 0;
        }


        overlapped_data connect_info;
        overlapped_data write_info;
        overlapped_data read_info;
        HANDLE          handle;
    };


    // Good references:
    // http://stackoverflow.com/questions/1672677/print-a-guid-variable
    // http://stackoverflow.com/questions/18555306/convert-guid-structure-to-lpcstr
    
    
    inline std::string new_guid()
    {
        GUID guid_struct = { 0 };
        ::CoCreateGuid(&guid_struct);
        OLECHAR olechar_guid[40] = { 0 };
        int nCount = ::StringFromGUID2(guid_struct, olechar_guid, 40);
        
        char guid_buf[1024];
        char *dst = guid_buf;
        OLECHAR *src = olechar_guid;
        char char1 = ' ';
        
        for (;; ++src)
        {
            char1 = (char)*src;
            
            if (char1 == '{')
                continue;

            if (char1 == '}')
            {
                *dst = 0;
                break;
            }

            *dst = char1;
            ++dst;
        }

        return guid_buf;
    }

    
    inline void new_guids(std::vector<std::string> &guids, int count)
    {
        ::CoInitialize(0);
        guids.clear();
        
        for (int i = 0; i < count; i++)
        {
            guids.push_back(new_guid());
        }
        
        ::CoUninitialize();
    }

    
    inline void new_pipe_names(std::vector<std::string> &pipe_names, const std::string &prefix, int count)
    {
        pipe_names.clear();
        std::vector<std::string> guids;
        new_guids(guids, count);

        for (std::string &guid : guids)
        {
            std::string name = "\\\\.\\pipe\\";
            name += prefix + '-' + guid;
            pipe_names.push_back(name);
        }
    }

    
    BOOL CreatePipe2(HANDLE *read_handle, HANDLE *write_handle)
    {
        std::vector<std::string> pipe_names;
        new_pipe_names(pipe_names, "pipe1", 1);
        std::string pipe_name = pipe_names[0];

        io_named_pipe pipe1;
        io_named_pipe pipe2;

        pipe1.listen(pipe_name);
        pipe2.connect(pipe_name);

        *read_handle = pipe1.handle;
        *write_handle = pipe2.handle;

        pipe1.detach_handle();
        pipe2.detach_handle();

        return TRUE;
    }
}

#endif

#endif

