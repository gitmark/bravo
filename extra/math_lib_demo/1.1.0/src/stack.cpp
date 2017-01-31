#include "stack.h"
#include "base.h"
#include "stack_p.h"

#include<memory>
#include<deque>
#include<vector>
#include<string>

using namespace std;

stack::stack() : base(*new stack_p)
{
    BP(stack)
    p->init();
}

stack::stack(stack_p &d) : base(d)
{
    BP(stack)
    p->init();
}

stack::~stack()
{
}

void stack::push(double num)
{
    BP(stack)
    p->push(num);
}

void stack::add()
{
    BP(stack)
    p->add();
}

void stack::mult()
{
    BP(stack)
    p->mult();
}

double stack::val()
{
    BP(stack)
    return p->val();
}

void stack::div()
{
    BP(stack)
    p->div();
}

//////////////////////////////////
// Private

void stack_p::init()
{
}

void stack_p::push(double num)
{
    nums.push_back(num);
}

void stack_p::add()
{
    func(op::add);
}

void stack_p::sub()
{
    func(op::sub);
}

void stack_p::mult()
{
    func(op::mult);
}

void stack_p::div()
{
    func(op::div);
}

double stack_p::val()
{
    return nums.back();
}

void stack_p::func(op _op)
{
    double d = nums.back();
    nums.pop_back();
    
    switch(_op)
    {
        case op::add:
        nums.back() += d;
        break;
        
        case op::sub:
        nums.back() -= d;
        break;
        
        case op::mult:
        nums.back() *= d;    // Fixed bug
        break;
        
        case op::div:
        nums.back() /= d;
        break;
    }
}












