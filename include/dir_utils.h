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

#ifndef DIR_UTILS_H_
#define DIR_UTILS_H_

#include <string>
#include <vector>
#include <fstream>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <Windows.h>


inline void read_dir(const std::string &path, const std::string &prefix, std::vector<std::string> &filenames)
{
    WIN32_FIND_DATA fd;

    std::string path2 = path + "/*";
    std::wstring path3(path2.begin(), path2.end());
    HANDLE hFind = ::FindFirstFile(path3.c_str(), &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) 
    {
        do 
        {
            //    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
            {
                std::wstring filename1 = fd.cFileName;
                std::string filename2(filename1.begin(), filename1.end());

                if (filename2.find(prefix) == 0)
                {
                    filenames.push_back(filename2);
                }
            }
        } while (::FindNextFile(hFind, &fd));

        ::FindClose(hFind);
    }
}


inline bool file_exists(const std::string &filename)
{
    std::wstring filename2(filename.begin(), filename.end());
    DWORD dwAttrib = GetFileAttributes(filename2.c_str());
    
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

#else

inline bool file_exists(const std::string &filename)
{
    return ( access( filename.c_str(), F_OK ) != -1 );
}

#endif


inline std::string to_win_path(std::string path)
{
    for (int i = 0; i < path.size(); i++)
    {
        if (path[i] == '/')
        {
            path[i] = '\\';
        }
    }

    return path;
}

// Good reference:
// http://stackoverflow.com/questions/18816126/c-read-the-whole-file-in-buffer

inline int read_file(const std::string &filename, std::vector<char> &buf, bool add_null = false)
{
    buf.clear();
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return -1;
    }

    std::streamsize len = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if(add_null)
    {
        buf.resize(len+1);
        file.read(buf.data(), len);
        buf[file.gcount()] = 0; // null terminator
    }
    else
    {
        buf.resize(len);
        file.read(buf.data(), len);
    }
    return 0;
}

}

#endif

