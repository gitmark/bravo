#ifndef CPP_TYPES_H
#define CPP_TYPES_H

#include <stdint.h>

namespace bravo
{

struct cpp_types
{

struct range_point
{
    int16_t table;
    int32_t key;
    int     valid;
    int16_t lt_value;
    int16_t eq_value;
};

     enum id { none, name, hex, integer, octal, char_int, flt, string, space, eol, ch, block_comment, line_comment, null_found };
     static const char* const id_to_string[];
     static const int         start_state = 1;
     static const int         state_count = 89;
     static const int         rule_count  = 16;
     static const int         table_size  = 256;
     static const int         next        [][ table_size   ];
     static const range_point next_ext    [];
     static const int         accept      [ state_count    ];
     static const int         null_trans  [ state_count    ];
     static const id          rule_to_id  [ rule_count     ];
};


}

#endif

