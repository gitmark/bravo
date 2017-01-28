#include "expression.h"
#include "expression_p.h"
#include "base.h"

#include<memory>
#include<deque>
#include<vector>
#include<string>
using namespace std;

class expression_p : public base_p
{
public:
    
    deque<double> args;
};

expression::expression()
{
    data = make_unique<expression_p>();

}