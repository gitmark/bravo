#ifndef RANGE_VALUE_H
#define RANGE_VALUE_H

#ifdef BRAVO_LIB_BUILD
#define LIB_PUBLIC __declspec(dllexport)
#else
#define LIB_PUBLIC
#endif

struct LIB_PUBLIC range_value
{
    range_value(int valid_ = 0,
                int16_t lt_value_ = 0,
                int16_t eq_value_ = 0) :
    valid(valid_),
    lt_value(lt_value_),
    eq_value(eq_value_)
    {}
    
    int valid = 0;
    int16_t lt_value = 0;
    int16_t eq_value = 0;
};


#endif
