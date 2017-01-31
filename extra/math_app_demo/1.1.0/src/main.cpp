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

int main(int argc, const char * argv[])
{
    stack s;
    s.push(2);
    s.push(3);
    s.add();

    int total = 0;
    int fail = 0;

    ++total;
    cout << "add: 2 + 3, " << s.val() << "\n";
    if (s.val() != 2.0 + 3.0)
    {
        ++fail;
        cout << "error: add returned " << s.val() << ", expected " << 2 + 5 << "\n";
    }

    s.push(4);
    s.push(5);
    s.mult();

    ++total;
    cout << "mult: 4 * 5, " << s.val() << "\n";
    if (s.val() != 4 * 5)
    {
        ++fail;
        cout << "error: mult() returned " << s.val() << ", expected " << 4 * 5 << "\n";
    }

    s.push(6);
    s.push(7);
    s.div();

    ++total;
    cout << "div: 6 / 7, " << s.val() << "\n";
    if (s.val() != 6.0 / 7.0)
    {
        ++fail;
        cout << "error: div() returned " << s.val() << ", expected " << 6.0 / 7.0 << "\n";
    }


    cout << "passed: " << total - fail << ", failed: " << fail << "\n";
    cout.flush();
    return 0;
}
