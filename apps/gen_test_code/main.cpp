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

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <iomanip>
#include <bravo/argv_parser.h>
#include <bravo/url.h>
#include <bravo/string_utils.h>
#include <bravo/hex.h>
using namespace std;
using namespace bravo;

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

int max_length(const vector<string> &names)
{
    int len = 0;

    for (string name : names)
    {
        if (name.length() > len)
            len = name.length();
    }

    return len;
}

void remove_empty(const vector<string> &vec1, vector<string> &result)
{
    for (auto part : vec1)
    {
        if (!part.empty())
            result.push_back(part);
    }
}

std::string gen_defines(const string &base, const vector<string> &names)
{
    stringstream ss;
    int num = 1;
    int max_len = max_length(names);
    for (auto name : names)
    {
        ss << "#define " << setw(max_len) << left << name << " (" << base << " + " << num << ")\n";
        ++num;
    }

    return ss.str();
}

std::string gen_ints_def(const vector<string> &names)
{
    stringstream ss;
    int num = 1;
    int max_len = max_length(names);
    for (auto name : names)
    {
        ss << "int " << setw(max_len) << left << name << " = 0;\n";
        ++num;
    }

    return ss.str();
}

std::string gen_assign_ints(const vector<string> &names)
{
    stringstream ss;

    int max_len = max_length(names);
    for (auto name : names)
    {
        ss << setw(max_len) << left << name << " = test_symbol_num(\"" << name << ");\n";
    }

    return ss.str();
}

std::string gen_test_meth(const vector<string> &names)
{
    stringstream ss;
    for (auto name : names)
    {
        ss << "TEST_METH(CLASSNUM + " << name << ")\n";
    }

    return ss.str();
}

std::string gen_nums(const vector<string> &names)
{
    stringstream ss;
    int max_len = max_length(names);
    for (auto name : names)
    {
        ss << setw(max_len + 8) << left << (string("nums[\"") + name + "\"]") << " = " << name << ";\\\n";
    }

    return ss.str();
}

std::string gen_method_names(const vector<string> &names)
{
    stringstream ss;
    int max_len = max_length(names);
    for (auto name : names)
    {
        ss << setw(max_len + 27) << left << (string("method_names()[CLASSNUM + ") + name + "]") << " = string(#COL_NAME) + \"::" << name << "\";\\\n";
    }

    return ss.str();
}



int main(int argc, const char *argv[])
{
    cmd_line.parse(argc,argv);
    
    std::string version = "0.0.0";
    
    if (cmd_line.flags.count("version"))
    {
        cout << "Exporter version " << version << "\n";
        return 0;
    }
    
    if (!cmd_line.args.size())
    {
        cout << cmd_line.error_msg();
        return 1;
    }

    std::string filename = cmd_line.args[0];
    ifstream file(filename);

    if (!file.is_open())
        return 0;

    std::string line;
    
    bool start = false;

    vector<string> names;
    string base;
    bool first = true;
    while (getline(file, line))
    {
        string var = trim(line);
        if (var.empty())
            continue;

        if (first)
        {
            base = var;
            first = false;
            continue;
        }
        names.push_back(var);
        cout << var << "\n";
    }
    cout << "\n";

    string defines = gen_defines(base, names);
    string ints_def = gen_ints_def(names);
    string assign_ints = gen_assign_ints(names);
    string test_meth = gen_test_meth(names);
    string nums = gen_nums(names);
    string method_names = gen_method_names(names);
    cout << defines << "\n";
    cout << ints_def << "\n";
    cout << assign_ints << "\n";
    cout << test_meth << "\n";
    cout << nums << "\n";
    cout << method_names << "\n";
    cout << "\n";
    std::string home;
    
#ifdef _WIN32
    home = std::getenv("USERPROFILE");
#else
    home = std::getenv("HOME");
#endif

    cout << "home: " << home << "\n";
    cout.flush();
    return 0;
}







