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
