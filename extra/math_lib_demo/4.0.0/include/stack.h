#ifndef STACK_H
#define STACK_H

#include <memory>
#include <base.h>

class stack_p;

class B_EXPORT stack : public base
{
public:
    stack();
    stack(stack_p &);
    ~stack();
    void push(double num);
    virtual void add();
	virtual void sub();
    void mult();
    double result();
    void div();
    
    double last_result;
    int stack_size;
};

#endif
