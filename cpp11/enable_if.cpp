/*
std::enable_if 是 C++ 标准库中的一个类模板，用于在满足某些条件时才调用一个函数。

std::enable_if 的定义如下：

template <bool Cond, class T = void>
struct enable_if {};


Cond 是判断是否满足条件的布尔表达式。T 是如果条件满足时返回的类型。
如果条件不满足，则 std::enable_if 没有任何类型。

std::enable_if 可以用于实现泛型编程，以及在编译时检测错误。
例如，以下代码使用 std::enable_if 来确保只有当 T 是可转换为 U 时才调用函数 f：


template <typename T, typename U>
typename std::enable_if<std::is_convertible<T, U>::value>::type
f(T t) {
  // 代码
}


如果 T 不是可转换为 U，则 f 将不会被调用。

std::enable_if 是一个强大的工具，可用于在编译时检查类型的属性。
std::enable_if 可以用于实现泛型编程，以及在编译时检测错误。

std::enable_if<true, int>::type t; // OK，定义了一个变量t，类型是int
std::enable_if<false, int>::type t2 // FAIL，因为没有type这个类型，编译失败
std::enable_if<true>::type; // OK, 第一模板参数是true，第二模板参数是通常版本中定义的默认类型即void
*/

#include <iostream>
#include <type_traits>
using namespace std;

// https://blog.csdn.net/zhuikefeng/article/details/107046375
namespace DemoA
{
    // 首先gt_zero是一个模板函数，typename std::enable_if<std::is_integral<T>::value,bool>::type是函数的返回值类型，
    // std::enable_if<>一般是由两部分组成，第一个参数是一个判定式，当判定式为真时，
    // 这个type数据成员存在，且值为bool，如果没有第二个参数，则默认值为void。

    // typename std::enable_if<!std::is_integral<T>::value, bool>::type 是一个类型特征，
    // 用于检查 T 是否不是整数类型。如果 T 不是整数类型，则类型特征的类型将是 bool。
    // 否则，类型特征的类型将是 void。

    // 如果判定式为假，type这个数据成员就是未定义的，更不存在值是什么。

    // 但是编译器并不会因为type值不存在就报错，因为泛型编程中存在一个最优匹配原则，这个模板不成立，就去推断其他模板，所以gt_zero实际上同时是一个重载函数。

    // 当第一个参数std::is_integral<T>::value为假时，!std::is_integral<T>::value肯定为真，
    // 所以第二个模板函数符合要求，同样std::enable_if<std::is_integral<T>::value,bool>::type的值为true。

    // is_integral：检查一个类型是否是整数类型。

    // gt_zero() 是一个模板函数，它有一个类型参数 T，表示传递给函数的值的类型。
    // 第二个类型参数 是一个类型特征，用于检查 T 是否不是整数类型。
    // 如果 T 不是整数类型，则函数将被调用。否则，函数将不会被调用。

    // cout << "is NOT integral"; 语句只会在 T 不是整数类型时执行。
    // return i > 0; 语句只会在 T 是整数类型且 i 大于 0 时执行。

    // 传入int型, 函数展开为: bool gt_zero(short int i, bool j = false)
    // 传非int型, 函数展开为: bool gt_zero(float i, void j = void())

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, bool>::type // 函数的返回值类型
    gt_zero(T i)
    {
        cout << "is integral";
        return i > 0;
    }

    // 传入int型: bool gt_zero(short int i, void j = void())
    // 传非int型: bool gt_zero(float i, bool j = false)
    template <typename T>
    typename std::enable_if<!std::is_integral<T>::value, bool>::type
    gt_zero(T i)
    {
        cout << "is NOT integral";
        return i > 0;
    }

    int Demo()
    {
        short int i = 2;
        float f = 3.1;
        std::cout << i << " greater than zero : " << gt_zero(i) << std::endl;
        std::cout << f << " greater than zero : " << gt_zero(f) << std::endl;
        return 0;
    }
}

namespace DemoB
{
    // 传入int型: bool gt_zero(int i, int j = 0)。
    // 传非int型: bool gt_zero(float i, void *j = nullptr)
    template <typename T>
    bool
    gt_zero(T i, typename std::enable_if<std::is_integral<T>::value, T>::type j = 0)
    {
        cout << "is integral";
        return i > 0;
    }

    // 传入int型: bool gt_zero(float i, void *j = nullptr)
    // 传非int型: bool gt_zero(int i, int j = 0)
    template <typename T>
    bool
    gt_zero(T i, typename std::enable_if<!std::is_integral<T>::value, T>::type j = 0)
    {
        cout << "is NOT integral";
        return i > 0;
    }

    int Demo()
    {
        short int i = 2;
        float f = 3.1;
        std::cout << i << " greater than zero : " << gt_zero(i) << std::endl;
        std::cout << f << " greater than zero : " << gt_zero(f) << std::endl;
        return 0;
    }
}

namespace DemoC
{
    // typename std::enable_if<std::is_integral<T>::value>::type 是一个类型特征，用于检查 T 是否是整数类型。
    // 如果 T 是整数类型，则该类型特征的类型将是 T, int * = nullptr。否则，该类型特征的类型将是 void, void *。
    // * 是解引用运算符。它用于访问指针的值。
    // nullptr 是一个特殊值，表示一个空指针。
    // 在这种情况下，代码表示 gt_zero() 函数中的 j 参数将是指向 T 类型值的指针，如果 T 是整数类型。否则，j 参数将是空指针。

    // 例如，如果 T 是 short int，那么 j 参数将是指向 short int 值的指针。但是，如果 T 是 float，那么 j 参数将是空指针。

    // 这段代码可用于确保 j 参数仅在它是 T 类型值的有效指针时才使用。
    // 如果 j 参数不是有效指针，那么 gt_zero() 函数将无法访问 j 的值并将返回错误。

    // 传入int型: bool gt_zero(int i)
    // 传非int型: void gt_zero(T i)
    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    bool
    gt_zero(T i)
    {
        cout << "is integral";
        return i > 0;
    }

    // 传入int型: void gt_zero(T i)
    // 传非int型  bool gt_zero(T i)
    template <typename T, typename std::enable_if<!std::is_integral<T>::value>::type * = nullptr>
    bool
    gt_zero(T i)
    {
        cout << "is NOT integral";
        return i > 0;
    }

    int Demo()
    {
        short int i = 2;
        float f = 3.1;
        std::cout << i << " greater than zero : " << gt_zero(i) << std::endl;
        std::cout << f << " greater than zero : " << gt_zero(f) << std::endl;
        return 0;
    }
}

int main()
{
    std::cout << "---- DemoA::Demo()" << std::endl;
    DemoA::Demo();

    std::cout << "---- DemoB::Demo()" << std::endl;
    DemoB::Demo();

    std::cout << "---- DemoC::Demo()" << std::endl;
    DemoC::Demo();

    return 0;
}

/*
---- DemoA::Demo()
2 greater than zero : is integral1
3.1 greater than zero : is NOT integral1
---- DemoB::Demo()
2 greater than zero : is integral1
3.1 greater than zero : is NOT integral1
---- DemoC::Demo()
2 greater than zero : is integral1
3.1 greater than zero : is NOT integral1
*/