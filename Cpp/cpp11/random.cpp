// c++ 11中的随机数 ——random_S大幕的博客-CSDN博客_c++11 rand
// https://blog.csdn.net/qq_34784753/article/details/79600809

//  在 C++ 11 标准中，定义在头文件 random 中的随机数库通过一组协作的类来解决这些问题，主要用到的是两个类：
// 随机数引擎类（random-number engines）
// 随机数分布类（random-number distribution)

// 其中，一个引擎类可以生成 unsigned 随机数列，一个分布使用一个引擎类生成指定类型的，在给定范围内的，服从指定概率分布的随机数。
// 随机数引擎是函数对象类，他们定义了一个调用运算符，该运算符不接受参数并返回一个随机的 unsigned 整数。
// 我们可以通过调用一个随机数引擎对象来生成原始随机数。

#include <iostream>
#include <random>

void unintFunc()
{
    std::cout << "\n---------------------------------" << std::endl;
    std::cout << " -> generation unsigned int num" << std::endl;
    // e() “调用”对象来生成下一个随机数
    std::default_random_engine e;    // 生成随机无符号数
    for (size_t i = 0; i < 10; i++)
        std::cout << e() << "\t";

    std::cout << "\n\n -> generation int num in range -100 ~ 100" << std::endl;
    std::uniform_int_distribution<int> u(-100, 100);

    for (size_t i = 0; i < 100; i++)
        std::cout << u(e) << "\t";

    std::cout << std::endl;
}

void floatFunc()
{
    std::cout << "\n---------------------------------" << std::endl;
    std::cout << " -> generation double num in range -1 ~ 1" << std::endl;
    std::default_random_engine eA;
    std::uniform_real_distribution<double >uA(-1, 1);
    for (size_t i = 0; i < 10; i++)
        std::cout << uA(eA) << "\t";

    std::cout << std::endl;

    std::cout << "\n\n -> generation float num in range -1 ~ 100" << std::endl;
    std::default_random_engine eB(time(0));
    std::uniform_real_distribution<float >uB(-1, 100);
    for (size_t i = 0; i < 30; i++)
        std::cout << uB(eB) << "\t";

    std::cout << std::endl;
}

int main()
{
    unintFunc();
    floatFunc();

    return 0;
}