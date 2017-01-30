#ifndef BASE_P_H
#define BASE_P_H

#include <base.h>

#define C_P(C) \
C##_p * p = static_cast<C##_p*>(p_.get());

class base_p
{
    public:
    void init();
    base *b;    
};

#endif

