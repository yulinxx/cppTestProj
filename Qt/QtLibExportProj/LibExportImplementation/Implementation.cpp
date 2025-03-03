#include "Implementation.h"

#include <iostream>

void Implementation::foo()
{
    std::cout << "foo function from Implementation." << std::endl;
    Interface::foo();
}

void Implementation::bar()
{
    std::cout << "bar function from Implementation." << std::endl;
}

double Implementation::add(double a, double b)
{
    return a + b;
}

double Implementation::sub(double a, double b)
{
    return a - b;
}

double Implementation::mul(double a, double b)
{
    return a * b;
}

double Implementation::div(double a, double b)
{
    return a / b;
}