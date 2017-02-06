//
//  main.cpp
//  math_app_demo
//
//  Created by Mark Elrod on 1/30/17.
//  Copyright © 2017 Mark Elrod. All rights reserved.
//

#include <iostream>

using namespace std;

#include <stack.h>

int test(double expected, double actual, const std::string& test_name, int &passed, int &failed)
{
    if (expected == actual)
    {
        cout << test_name << "\n";
        ++passed;
        return 0;
    }
    
    ++failed;
    cout << "failed: " << test_name << ", expected: " << expected << ", actual: " << actual << "\n";
    
    return 0;
}

int main(int argc, const char * argv[])
{
    stack s;
    s.push(2);
    s.push(3);
    s.add();

    cout << "Test Summary\n";
    int passed = 0;
    int failed = 0;
    test(2.0 + 3.0, s.result(), "add:  2 + 3", passed, failed);


    s.push(4);
    s.push(5);
    s.mult();

    test(4.0 * 5.0, s.result(), "mult: 4 * 5", passed, failed);

    s.push(6);
    s.push(7);
    s.div();

    test(6.0 / 7.0, s.result(), "div:  6 / 7", passed, failed);

    s.push(8);
    s.push(9);
    s.sub();

    test(8.0 - 9.0, s.result(), "sub:  8 - 9", passed, failed);
    test(8.0 - 9.0, s.last_result, "last_result: 8 - 9", passed, failed);
    test(4, s.stack_size, "stack_size:", passed, failed);
    
    
    

cout << "----------------------------\n";
    cout << "passed: " << passed << ", failed: " << failed << "\n";
    
    int delta = (long long)(&s.stack_size) - (long long)&s.last_result;
    
    cout << "delta: " << delta << "\n";
    
    
    cout.flush();
    return 0;
}
