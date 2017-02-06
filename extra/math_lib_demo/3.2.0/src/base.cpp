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
_p(new base_p)
{
    BP(base)
    p->init();
    p->b = this;
    valid = true;
}

base::base(base_p &b) :
    _p(&b)
{
    BP(base)
    p->init();
    p->b = this;
    valid = true;
}

base::~base()
{
}

void base_p::init()
{
}

