#include "expression.h"
#include "expression_p.h"
#include "base.h"

#include<memory>
#include<deque>
#include<vector>
#include<string>
using namespace std;

void expression_p::init()
{
    
}

void expression_p::push(double num)
{
    nums.push_back(num);
}

expression::expression(expression_p &d) : base(d)
{
    C_P(expression)
    p->init();
}

expression::expression() : base(*new expression_p)
{
    C_P(expression)
    p->init();
}

void expression::push(double num)
{
    C_P(expression)
    p->push(num);
}
