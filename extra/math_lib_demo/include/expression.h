#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>
#include <base.h>

class expression_p;

class expression : public base
{
public:
    expression();
    expression(expression_p &);
    
    void push(double num);
    double add();
    double mult();
};

#endif
