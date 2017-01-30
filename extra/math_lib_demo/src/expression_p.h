#ifndef EXPRESSION_P_H
#define EXPRESSION_P_H

#include "base_p.h"
#include <deque>

class expression_p : public base_p
{
    public:
    void init();
    void push(double d);
    
    std::deque<double> nums;
};

#endif
