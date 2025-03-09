#include <iostream>

// constexpr 和 const 都是 C++ 中用于定义常量的关键字,但它们在语义和用法上有一些区别.

// 1. const:
// - const 用于声明一个不可修改的变量.它表示该变量的值在声明后不能被修改.
// - const 变量可以在编译时或运行时初始化.
// - const 变量的值在运行时是确定的,但编译时可能不确定.
// - const 变量的值可以通过函数调用、运行时计算或其他动态方式确定.
// - const 变量通常用于表示不可变的数据.

// 示例:
const int num = 10;
const float pi = 3.14;

void foo(const int value)
{
    // 无法修改 value 的值
}

// 2. constexpr:
// - constexpr 用于声明在编译时求值的常量表达式.它表示该常量在编译时就可以被计算出来,并且可以在编译时用于编译器优化.
// - constexpr 变量必须在声明时进行初始化,并且初始化表达式必须是一个常量表达式.
// - constexpr 变量的值在编译时是确定的,并且可以在编译时被计算.
// - constexpr 变量通常用于表示在编译时已知且不会改变的值.

// 示例:
// constexpr int square(int value) {
//     return value * value;
// }

// 总结:
// - const 用于声明不可修改的变量,值在运行时确定.
// - constexpr 用于声明在编译时可以确定值的常量,值在编译时计算.

// 需要注意的是,constexpr 在 C++11 中引入,支持更多的编译时计算和编译器优化,而 const 在更早的版本中就存在.
// 在使用常量时,根据具体需求选择适当的关键字.

// 下面是一些使用 constexpr 的示例:

// 1. 计算编译时常量:
constexpr int square(int value)
{
    return value * value;
}

// 在这个示例中,square() 函数被声明为 constexpr,它接受一个整数参数并返回参数的平方.
// 然后,使用 constexpr 关键字创建了一个常量 num,并将 square(5) 的结果赋值给它.
// 由于参数和返回值都是常量表达式,所以可以在编译时计算出结果,并将其作为常量存储.

// 2. 字符串字面量的编译时计算:

constexpr int stringLength(const char* str)
{
    int length = 0;
    while (*str != '\0')
    {
        ++length;
        ++str;
    }
    return length;
}

constexpr int len = stringLength("Hello"); // 在编译时计算出字符串长度

// 在这个示例中,stringLength() 函数接受一个字符串字面量的指针,并计算字符串的长度.
// 由于字符串字面量在编译时是已知的,因此可以使用 constexpr 关键字使 stringLength("Hello")
// 的结果在编译时计算出字符串的长度.

// 3. 递归的编译时计算:

constexpr int factorial(int n)
{
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

int main()
{
    constexpr int num = square(5);  // 在编译时计算出常量值

    constexpr int result = factorial(5); // 在编译时计算出阶乘值

    // 在这个示例中,factorial() 函数使用递归方式计算一个整数的阶乘.
    // 通过使用 constexpr 关键字,可以在编译时展开递归调用,最终计算出整数的阶乘,并将其作为常量存储.

    // 需要注意的是,constexpr 变量和函数应该满足编译时计算的要求,其中所有的输入和操作都必须是编译时可确定的.
    // 编译器对于 constexpr 的使用有一些限制和规定,因此在使用 constexpr 时要遵循相关的规则.

    // 通过使用 constexpr,我们可以在编译时计算出常量值,提供更高的性能和编译器优化,
    // 并在一些情况下提供更灵活的代码编写方式.

    return 0;
}