#ifndef stack_P_H
#define stack_P_H

#include "base_p.h"
#include <vector>


class stack_p : public base_p
{
    enum class op {add, sub, mult, div};
    public:
    void init();
    void push(double d);
    void add();
    void sub();
    void mult();
    void div();
    double val();
    void func(op _op);
    
    std::vector<double> nums;
};

#endif
