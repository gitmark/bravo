#include "expression.h"
#include "expression_p.h"
#include "base.h"
#include "base_p.h"

#include<memory>
#include<deque>
#include<vector>
#include<string>
using namespace std;


void base_p::init()
{
    
}

base::base() :
p_(new base_p)
{
    C_P(base)
    p->init();
    p->b = this;
}

base::base(base_p &b) :
    p_(&b)
{
    C_P(base)
    p->init();
    p->b = this;
}

