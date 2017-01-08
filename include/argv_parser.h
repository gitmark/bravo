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

#ifndef ARGV_PARSER_H
#define ARGV_PARSER_H

#include <map>
#include <string>
#include <vector>

#define AP_SUCCESS        0
#define AP_NO_ARG         1
#define AP_REQUIRED_ARG   2
#define AP_OPTIONAL_ARG   3
#define AP_NOT_A_FLAG     4
#define AP_UNKNOWN_FLAG   5
#define AP_SINGLE_DASH    6
#define AP_DOUBLE_DASH    7
#define AP_VERIFY_ERROR   8


namespace bravo
{
    
extern const char *ap_strings[];

int get_flags(std::map<std::string, int>                flag_defs,
                const std::string &                     arg,
                std::map<std::string, std::string> &    flags,
                std::string &                           last_flag);

int get_flags(int argc, const char *argv[],
                std::map<std::string, int>              flag_defs,
                std::map<std::string, std::string> &    flags,
                int &                                   index);


class argv_parser
{
public:
    
    argv_parser()
    {
        error_      = AP_SUCCESS;
        verbose_    = false;
    }

    virtual ~argv_parser() {}

    virtual int verify()
    {
        // Set by derived class
        return error_;
    }

    int parse_(int argc, const char* argv[],
               std::map<std::string, std::string> &flags,
               std::vector<std::string> &args)
    {
        int index = 0;
        int state = get_flags(argc, argv, flag_defs, flags, index);
        error_ = AP_SUCCESS;
        error_string_.clear();

        if (AP_REQUIRED_ARG == state || AP_UNKNOWN_FLAG == state || AP_VERIFY_ERROR == state)
        {
            error_string_ = std::string("error: ") + ap_strings[state];

            if (index < argc)
            {
                error_string_ += std::string(", ") + argv[index];
            }

            error_ = state;
        }
        else
        {
            for (int i = index; i < argc; ++i)
                args.push_back(argv[i]);

            // Set error_ and error_string
            verify();
        }

        return error_;
    }

    int parse(int argc, const char* argv[])
    {
        return parse_(argc, argv, flags, args);
    }
    
    inline int error() { return error_; }

    inline std::string usage() { return usage_; }
    
    inline std::string error_msg()
    {
        return error_string_ + "\n" + usage_;
    }

    inline bool verbose() { return verbose_; }
    
protected:
    
    std::string error_string_;
    std::string usage_;
    int         error_;
    bool        verbose_;
    std::map<std::string, int>          flag_defs;
    
public:
    
    std::map<std::string, std::string>  flags;
    std::vector<std::string>            args;
};

}

#endif

