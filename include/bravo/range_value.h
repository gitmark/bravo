#ifndef RANGE_VALUE_H
#define RANGE_VALUE_H

struct range_value
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
