// // Boost.TypeTraits是一个C++库,它提供了用于类型特性的一组模板类和函数.
// // 它可以用来在编译时查询和操作类型信息,例如判断类型是否是指针类型、是否是整数类型,以及获取类型的常量性和引用性等.

// // 以下是一些使用Boost.TypeTraits的示例:

// // 1. 检查类型是否为指针类型:

// #include <boost/type_traits.hpp>

// template<typename T>
// void checkPointerType(T t)
// {
//     if(boost::is_pointer<T>::value)
//         std::cout << "T is a pointer type" << std::endl;
//     else
//         std::cout << "T is not a pointer type" << std::endl;
// }

// int main()
// {
//     int* ptr = nullptr;
//     checkPointerType(ptr); // 输出 "T is a pointer type"

//     int num = 10;
//     checkPointerType(num); // 输出 "T is not a pointer type"

//     return 0;
// }

// // 2. 获取类型的常量性:

// #include <boost/type_traits.hpp>

// template<typename T>
// void checkConstness(T t)
// {
//     if(boost::is_const<T>::value)
//         std::cout << "T is a const type" << std::endl;
//     else
//         std::cout << "T is not a const type" << std::endl;
// }

// int main()
// {
//     const int num = 10;
//     checkConstness(num); // 输出 "T is a const type"

//     int num2 = 20;
//     checkConstness(num2); // 输出 "T is not a const type"

//     return 0;
// }

// // 3. 判断类型是否为整数类型:

// #include <boost/type_traits.hpp>

// template<typename T>
// void checkIntegerType(T t)
// {
//     if(boost::is_integral<T>::value)
//         std::cout << "T is an integer type" << std::endl;
//     else
//         std::cout << "T is not an integer type" << std::endl;
// }

// int main()
// {
//     int num = 10;
//     checkIntegerType(num); // 输出 "T is an integer type"

//     double val = 3.14;
//     checkIntegerType(val); // 输出 "T is not an integer type"

//     return 0;
// }

// // 这些示例只是Boost.TypeTraits库提供的功能的一小部分.
// // Boost.TypeTraits还提供了许多其他有用的类型特性查询和转换功能,例如判断类型是否可调用、类型的成员函数指针类型等.
// // 你可以参考Boost.TypeTraits文档以获取更详细的信息和更多示例:https://www.boost.org/doc/libs/1_76_0/libs/type_traits/doc/html/index.html

#include <iostream>
#include <boost/type_traits.hpp>

class MyClass
{
    // 自定义类
};

struct MyStruct
{
    // 自定义结构体
};

int main()
{
    if (boost::is_class<MyClass>::value)
    {
        std::cout << "MyClass is a class" << std::endl;
    }
    else
    {
        std::cout << "MyClass is not a class" << std::endl;
    }

    if (boost::is_class<MyStruct>::value)
    {
        std::cout << "MyStruct is a class" << std::endl;
    }
    else
    {
        std::cout << "MyStruct is not a class" << std::endl;
    }

    return 0;
}