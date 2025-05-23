// std::bind 是 C++ 标准库中的一个函数模板,用于创建函数对象(函数绑定).

#include <functional>
#include <iostream>

// 函数原型
void foo(int a, int b, int c)
{
    std::cout << "a: " << a << " b: " << b << " c: " << c << std::endl;
}

class Foo
{
public:
    void bar(int a, int b)
    {
        std::cout << "a + b = " << a + b << std::endl;
    }
};

template<typename T, typename M>
int compair(T* begin, T* end, M func)
{
    int count = 0;

    for (; begin != end; begin++)
    {
        if (func(*begin))
            count++;
    }

    return count;
}

int main()
{
    /////////////////////////////////
    // 使用 std::bind 创建一个函数对象,绑定了参数 a 和 b
    // std::bind 用于创建一个函数对象 boundFunc,它绑定了函数 foo 的参数 a 和 b,
    // 并使用占位符 std::placeholders::_1 表示参数 ,以便在稍后调用 boundFunc 时传入.

    auto boundFunc = std::bind(foo, 10, 20, std::placeholders::_1);

    // 调用函数对象,传入参数 c
    // 当调用 boundFunc(30) 时,实际上会调用函数 foo(10, 20, 30).
    boundFunc(30);

    /////////////////////////////////
    // 使用 std::bind 绑定成员函数和对象
    // std::bind 用于创建一个函数对象 boundFunc,它绑定了成员函数 bar 和对象 obj.
    // 参数 a 被占位符 std::placeholders::_1 表示,而参数 b 则被固定为 100.

    // 当调用 boundFunc(42) 时,实际上会调用 obj.bar(42, 100),输出结果为 a + b = 142.
    Foo obj;
    auto boundFunc2 = std::bind(&Foo::bar, &obj, std::placeholders::_1, 100);

    // 调用函数对象,传入参数 a
    boundFunc2(42);

    //// C++函数的进化 函数→函数指针→函数模板→仿函数|函数对象→lambda表达式
    // https://www.bilibili.com/video/BV1zo4y1N7No/
    int nArray[] = { 2, 32, 13, 55, 23, 6, 10, 13 };
    int nSize = sizeof(nArray) / sizeof(int);

    auto func = [](auto& val) {
        return val > 20;
        };

    int nRes = compair(nArray, nArray + nSize, func);
    std::cout << "Size is :" << nRes << std::endl;

    return 0;
}