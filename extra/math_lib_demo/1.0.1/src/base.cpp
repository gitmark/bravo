#include "stack.h"
#include "base.h"
#include "stack_p.h"
#include "base_p.h"

#include<memory>
#include<deque>
#include<vector>
#include<string>

using namespace std;

base::base() :
p_(new base_p)
{
    BP(base)
    p->init();
    p->b = this;
}

base::base(base_p &b) :
    p_(&b)
{
    BP(base)
    p->init();
    p->b = this;
}

base::~base()
{
    delete p_;
    p_ = nullptr;
}

void base_p::init()
{
}

