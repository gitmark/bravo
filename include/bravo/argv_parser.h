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

#define LIB_LOCAL 
// __attribute__((visibility("hidden")))
namespace bravo
{

static const char *ap_strings[] = {
    "AP_SUCCESS",
    "AP_NO_ARG",
    "AP_REQUIRED_ARG",
    "AP_OPTIONAL_ARG",
    "AP_NOT_A_FLAG",
    "AP_UNKNOWN_FLAG",
    "AP_SINGLE_DASH",
    "AP_DOUBLE_DASH",
    "AP_VERIFY_ERROR" };

inline LIB_LOCAL int get_flags(std::map<std::string, int>                flag_defs,
                const std::string &                     arg,
                std::map<std::string, std::string> &    flags,
                std::string &                           last_flag);

inline LIB_LOCAL int get_flags(int argc, const char *argv[],
                std::map<std::string, int>              flag_defs,
                std::map<std::string, std::string> &    flags,
                int &                                   index);

class LIB_LOCAL argv_parser
{
public:
    
    inline argv_parser()
    {
        error_      = AP_SUCCESS;
        verbose_    = false;
    }

    inline virtual ~argv_parser() {}

    inline virtual int verify()
    {
        // Set by derived class
        return error_;
    }

    inline int parse_(int argc, const char* argv[],
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
                error_string_ += std::string(", ") + argv[index];

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

    inline int parse(int argc, const char* argv[])
    {
        return parse_(argc, argv, flags, args);
    }
    
    inline std::string usage() { return usage_; }
    
    inline std::string error_msg()
    {
        return error_string_ + "\n" + usage_ + "\n";
    }
    
    inline int error() { return error_; }

    inline bool verbose() { return verbose_; }
    
protected:
    
    std::string usage_;
    std::string error_string_;
    int         error_;
    bool        verbose_;
    std::map<std::string, int>          flag_defs;
    
public:
    
    std::map<std::string, std::string>  flags;
    std::vector<std::string>            args;
};




inline int get_flags(std::map<std::string, int>         flag_defs,
    const std::string &         arg,
    std::map<std::string, std::string> &  flags,
    std::string &               last_flag)
{
    last_flag.clear();

    if (arg.empty())
        return AP_NOT_A_FLAG;

    int state = AP_NOT_A_FLAG;

    if (arg[0] == '-')
    {
        state = AP_SINGLE_DASH;

        for (int i = 1; i < arg.size(); ++i)
        {
            if (AP_DOUBLE_DASH == state)
            {
                std::string flag;
                size_t equals_pos = arg.find("=");

                if (equals_pos == std::string::npos)
                {
                    flag = arg.substr(i);

                    if (!flag_defs.count(flag))
                    {
                        last_flag.clear();
                        return AP_UNKNOWN_FLAG;
                    }

                    flags[flag];
                    last_flag = flag;
                    state = flag_defs[flag];
                    return state;
                }
                else
                {
                    flag = arg.substr(i, equals_pos - i);

                    if (!flag_defs.count(flag))
                    {
                        last_flag.clear();
                        return AP_UNKNOWN_FLAG;
                    }

                    if (arg.size() - 1 > equals_pos)
                        flags[flag] = arg.substr(equals_pos + 1);
                    else
                        flags[flag];

                    last_flag = "";
                    return AP_NO_ARG;
                }
            }
            else if (state == AP_OPTIONAL_ARG || state == AP_REQUIRED_ARG)
            {
                flags[last_flag] = arg.substr(i);
                return AP_SUCCESS;
            }
            else
            {
                char ch = arg[i];

                if (ch == '-')
                    state = AP_DOUBLE_DASH;
                else
                {
                    std::string flag;
                    flag += arg[i];

                    if (!flag_defs.count(flag))
                    {
                        last_flag.clear();
                        return AP_UNKNOWN_FLAG;
                    }
                    else
                    {
                        flags[flag]; // Enter a place holder
                        state = flag_defs[flag];
                        last_flag = flag;
                    }
                }
            }
        }
    }

    return state;
}

inline int get_flags(int argc, const char *argv[],
    std::map<std::string, int>         flag_defs,
    std::map<std::string, std::string> &    flag_vals,
    int &                         index)
{
    int state = AP_SUCCESS;
    std::string last_flag;
    int i = 1;

    // The value of i when this for loop exits is important. i is assigned to index which the caller may use.
    // index indicates the location of the first parameter after the flags.
    for (; i < argc; ++i)
    {
        // Double dash indicates there are no more flags
        if (state == AP_DOUBLE_DASH)
            break;

        if (state == AP_REQUIRED_ARG)
        {
            flag_vals[last_flag] = argv[i];
            state = AP_NO_ARG;
        }
        else
            state = get_flags(flag_defs, argv[i], flag_vals, last_flag);

        // A single dash by itself is treated as an argument and indicates that standard io should be used.
        if (state == AP_NOT_A_FLAG || state == AP_UNKNOWN_FLAG || state == AP_SINGLE_DASH)
            break;
    }

    index = i;
    return state;
}

}

#endif

