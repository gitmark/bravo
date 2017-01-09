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
********************************************************************************/
 
#include "argv_parser.h"

using namespace std;

namespace bravo
{
const char *ap_strings[] = {
    "AP_SUCCESS",
    "AP_NO_ARG",
    "AP_REQUIRED_ARG",
    "AP_OPTIONAL_ARG",
    "AP_NOT_A_FLAG",
    "AP_UNKNOWN_FLAG",
    "AP_SINGLE_DASH",
    "AP_DOUBLE_DASH",
    "AP_VERIFY_ERROR" };

int get_flags(std::map<string, int>         flag_defs,
                const std::string &         arg,
                std::map<string, string> &  flags,
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

int get_flags(int argc, const char *argv[],
              std::map<string, int>         flag_defs,
              std::map<string, string> &    flag_vals,
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


