#include <bravo/argv_parser.h>

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

int main(int argc, const char *argv[])
{
    cmd_line.parse(argc,argv);
    
    return 0;
}
