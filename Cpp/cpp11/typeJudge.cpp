#include <iostream>
#include <type_traits>

union A
{
};
struct B
{
};
enum C
{
};
class D
{
};

template<typename T>
void foo()
{
    std::cout << std::is_class<T>::value << std::endl;
}

int main()
{
    std::cout << "C is class ?: ";
    foo<C>();
    std::cout << "B is class ?: ";
    foo<B>();
    std::cout << "D is class ?: " << std::is_class<D>::value << std::endl;
    std::cout << "A is union ?: " << std::is_union<A>::value << std::endl;
    C c;
    std::cout << "c is enum ?: " << std::is_enum<decltype(c)>::value << std::endl;
    return 0;
}