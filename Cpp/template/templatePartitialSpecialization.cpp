// 模板偏特化(Template Partitial Specialization)

// 偏特化又叫部分特化,既然是「部分」的,那么就不会像全特化那样直接实例化了.
// 偏特化的模板本质上还是模板,它仍然需要编译期来根据需要进行实例化的,
// 所以,在链接方式上来说,全特化要按普通函数/类/变量来处理,而偏特化模板要按模板来处理.

// 模板函数不支持偏特化,因此偏特化讨论的主要是模板类.

#include <iostream>

namespace A
{
    template <typename T1, typename T2>
    struct Test
    {
        Test()
        {
            std::cout << "Test" << std::endl;
        }
    };

    template <typename T>
    struct Test<int, T>
    {
        Test(int, T)
        {
            std::cout << "Test<int, T>" << std::endl;
        }
        static void f();
    };

    template <typename T>
    void Test<int, T>::f()
    {
        std::cout << "Test<int, T>::f()" << std::endl;
    }

    void Demo()
    {
        Test<int, int>::f();    // 按照偏特化实例化,有f函数
        Test<int, double>::f(); // 按照偏特化实例化,有f函数
        //   Test<double, int>::f(); // 按照通用模板实例化,不存在f函数,编译报错

        // 偏特化模板本身仍然是模板,仍然需要经历实例化.
        // 但偏特化模板可以指定当一些参数满足条件时,应当按照指定方式进行实例化而不是通用模板定义的方式来实例化.
    }
}

// 那如果偏特化和全特化同时存在呢？比如下面的情况:
namespace B
{
    template <typename T1, typename T2>
    struct Test
    {
        Test()
        {
            std::cout << "Test A" << std::endl;
        }
    }; // [0]通用模板

    template <typename T>
    struct Test<int, T>
    {
        Test()
        {
            std::cout << "Test B" << std::endl;
        }
    }; // [1]偏特化模板

    template <>
    struct Test<int, int>
    {
        Test()
        {
            std::cout << "Test C" << std::endl;
        }
    }; // [2]全特化模板

    void Demo()
    {
        Test<int, int> t; // 按照哪个实例化？
        // 上面的实例会按照[2]的方式,也就是直接调用全特化.大致上来说,全特化优先级高于偏特化,偏特化高于通用模板.
    }
}

// 对于函数来说,模板函数不支持偏特化,但支持重载,并且重载的优先级高于全特化.比如:
namespace C
{
    void f(int a, int b)
    {
        std::cout << "f a" << std::endl;
    } // 重载函数

    template <typename T1, typename T2>
    void f(T1 a, T2 b)
    {
        std::cout << "f b" << std::endl;
    }; // 通用模板

    template <>
    void f<int, int>(int a, int b)
    {
        std::cout << "f c" << std::endl;
    } // 全特化

    void Demo()
    {
        f(1, 2);     // 会调用重载函数
        f<>(1, 2);   // 会调用全特化函数f<int, int>
        f(2.5, 2.6); // 会用通用模板生成f<double, double>
    }
}

// 回到模板类的偏特化上,除了上面那种制定某些参数的偏特化以外,还有一种相对复杂的偏特化,请看示例:
namespace D
{
    template <typename T>
    struct Tool
    {
        Tool()
        {
            std::cout << "Tool" << std::endl;
        }
    }; // 这是另一个普通的模板类

    template <typename T>
    struct Test
    {
        Test()
        {
            std::cout << "Test" << std::endl;
        }
    }; // [0]通用模板

    template <typename T>
    struct Test<Tool<T>>
    {
        Test()
        {
            std::cout << "Test<Tool<T>>" << std::endl;
        }
    }; // [1]偏特化

    void Demo()
    {
        Test<int> t1;          // 使用[0]实例化Test<int>
        Test<Tool<int>> t2;    // 使用[1]实例化Test<Tool<int>>
        Test<Tool<double>> t3; // 使用[1]实例化Test<Tool<double>>
    }
}

// 有的资料会管上面这种特化叫做「模式特化」,用于区分普通的「部分特化」.
// 但它们其实都属于偏特化的一种,因为偏特化都是相当于特化了参数的范围.在上面的例子中,我们是针对于「参数是Tool的实例类型」这种情况进行了特化.

// 所以,偏特化并不一定意味着模板参数数量变小,它有可能不变,甚至有可能是增加的,比如说:
namespace E
{
    template <typename T1, typename T2>
    struct Tool
    {
        Tool()
        {
            std::cout << "Tool" << std::endl;
        }
    }; // 这是另一个普通的模板类

    template <typename T>
    struct Test
    {
        Test()
        {
            std::cout << "Test" << std::endl;
        }
    }; // [0]通用模板

    template <typename T1, typename T2>
    struct Test<Tool<T1, T2>>
    {
        Test()
        {
            std::cout << "Test<Tool<T1, T2>>" << std::endl;
        }
    }; // [1]偏特化模板

    template <typename T>
    struct Test<Tool<int, T>>
    {
        Test()
        {
            std::cout << "Test<Tool<int, T>>>" << std::endl;
        }
    }; // [2]偏特化模板

    void Demo()
    {
        Test<int> t1;               // [0]
        Test<Tool<int, double>> t2; // [2]
        Test<Tool<double, int>> t3; // [1]
    }
}

// 所以偏特化的引入,让模板编程这件事有了爆炸性的颠覆,因为其中的组合可以随意发挥想象.
// 但这里就引入了另一个问题,就比如上例中,[1]和[2]都是偏特化的一种,
// 但为什么Test<Tool<int, double>>选择了[2]而不是[1]呢？这么说,
// 看来不仅仅是跟全特化和通用模板存在优先级问题,多种偏特化之间也仍然存在优先级问题

int main()
{
    std::cout << "Template Partitial Specialization" << std::endl;

    std::cout << "--- A::Demo" << std::endl;
    A::Demo();

    std::cout << "--- B::Demo" << std::endl;
    B::Demo();

    std::cout << "--- C::Demo" << std::endl;
    C::Demo();

    std::cout << "--- D::Demo" << std::endl;
    D::Demo();

    std::cout << "--- E::Demo" << std::endl;
    E::Demo();

    std::cout << "---- END ---" << std::endl;

    // 输出
    // Template Partitial Specialization
    //--- A::Demo
    // Test<int, T>::f()
    // Test<int, T>::f()
    // --- B::Demo
    // Test C
    // --- C::Demo
    // f a
    // f c
    // f b
    // --- D::Demo
    // Test
    // Test<Tool<T>>
    // Test<Tool<T>>
    // --- E::Demo
    // Test
    // Test<Tool<int, T>>>
    // Test<Tool<T1, T2>>
    // ---- END ---
    return 0;
}