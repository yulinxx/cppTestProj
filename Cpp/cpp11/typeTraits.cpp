// type_traits是C++11提供的模板元基础库.
// type_traits可实现在编译期计算、判断、转换、查询等等功能.
// type_traits提供了编译期的true和false.

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

// 判断类型是否为指针类型
template<typename T>
void check_pointer_type(T value)
{
    if (std::is_pointer<T>::value)
    {
        // 是指针类型
        std::cout << "Pointer" << std::endl;
    }
    else
    {
        // 不是指针类型
        std::cout << "Not Pointer" << std::endl;
    }
}

template<typename T>
void checkFloatingPointType()
{
    if (std::is_floating_point<T>::value)
    {
        // 类型是浮点类型
    }
    else
    {
        // 类型不是浮点类型
    }
}

class Base
{
};

class Derived : public Base
{
};

template<typename BaseClass, typename DerivedClass>
void checkDerivedClass()
{
    if (std::is_base_of<BaseClass, DerivedClass>::value)
    {
        std::cout << "DerivedClass Is BaseClass subclass" << std::endl;
    }
    else
    {
        std::cout << "DerivedClass Not BaseClass subclass" << std::endl;
    }
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

    int i(10);//i是左值  有内存实体
    int& ri(i);
    int&& rri(i + 5);//右值引用
    std::cout << std::is_lvalue_reference<decltype(i)>::value << std::endl;//是左值不是引用 输出0
    std::cout << std::is_lvalue_reference<decltype(ri)>::value << std::endl;//是左值引用输出1
    std::cout << std::is_lvalue_reference<decltype(rri)>::value << std::endl;//是右值引用输出0

    // 判断int和const int类型
    std::cout << "int: " << std::is_const<int>::value << std::endl;
    std::cout << "const int:" << std::is_const<const int>::value << std::endl;

    // 判断类型是否相同
    std::cout << std::is_same<int, int>::value << "\n";
    std::cout << std::is_same<int, unsigned int>::value << "\n";

    // 添加/移除const
    std::cout << std::is_same<const int, std::add_const<int>::type>::value << std::endl;
    std::cout << std::is_same<int, std::remove_const<const int>::type>::value << std::endl;

    //添加引用
    std::cout << std::is_same<int&, std::add_lvalue_reference<int>::type>::value << std::endl;
    std::cout << std::is_same<int&&, std::add_rvalue_reference<int>::type>::value << std::endl;

    // 取公共类型
    typedef std::common_type<unsigned char, short, int>::type NumericType;
    std::cout << std::is_same<int, NumericType>::value << std::endl;

    checkDerivedClass<Base, Derived>();  // 输出:DerivedClass是BaseClass的子类或派生类
    checkDerivedClass<Derived, Base>();  // 输出:DerivedClass不是BaseClass的子类或派生类

    return 0;
}