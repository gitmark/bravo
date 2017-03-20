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

void remove_empty(const vector<string> &vec1, vector<string> &result)
{
    for (auto part : vec1)
    {
        if (!part.empty())
            result.push_back(part);
    }
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
/*    
    if (!cmd_line.args.size())
    {
        cout << cmd_line.error_msg();
        return 1;
    }
    */

    ifstream file("math-lib-demo-exports.txt");

    if (!file.is_open())
        return 0;

    std::string line;
    
    bool start = false;

    while (getline(file, line))
    {
        vector<string> parts1;
        bravo::split(line, " ", parts1);

        vector<string> parts2;
        remove_empty(parts1, parts2);
         
        if (parts2.size() == 4 && parts2[0] == "ordinal"
            && parts2[1] == "hint"
            && parts2[2] == "RVA"
            && parts2[3] == "name")
        {
            getline(file, line); // read empty line
            break;
        }
    }

    std::vector<std::vector<string>> db;
    while (getline(file, line))
    {
        vector<string> parts1;
        bravo::split(line, " ", parts1);

        vector<string> parts2;
        remove_empty(parts1, parts2);

        if (parts2.size() < 3)
            break;

        db.push_back(parts2);
    }
    
    std::string library = "math_lib_demo";
    cout << "LIBRARY " << library << "\n";
    cout << "EXPORTS\n";
    int i = 1;
    for (auto parts : db)
    {
        std::string name = parts[3];
        cout << "    " << name << " @" << i << "\n";
        cout.flush();
        ++i;
    }

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







