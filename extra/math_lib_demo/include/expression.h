#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <memory>
#include <base.h>

class base_p;

class expression : public base
{
public:
    expression();

    std::unique_ptr<base_p> data;
};

#endif
