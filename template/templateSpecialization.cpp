// 模板特化（template specialization）
// 模板偏特化（Template Partitial Specialization）

// 模板参数在某种特定类型下的具体实现称为模板特化。
// 模板特化有时也称之为模板的具体化，分别有函数模板特化和类模板特化。

// 模板偏特化（Template Partitial Specialization）是模板特化的一种特殊情况，
// 指显示指定部分模板参数而非全部模板参数，或者指定模板参数的部分特性分而非全部特性，也称为模板部分特化。
// 与模板偏特化相对的是模板全特化，指对所有模板参数进行特化。模板全特化与模板偏特化共同组成模板特化。
// 模板偏特化主要分为两种，一种是指对部分模板参数进行全特化，
// 另一种是对模板参数特性进行特化，包括将模板参数特化为指针、引用或是另外一个模板类。

// 模板参数的自动推导主要分为3种：
// 根据函数参数自动推导（模板函数）
// 根据构造参数自动推导（模板类）
// 自定义构造推导（模板类）
// 自动推导只能推导类型参数，而整数和整数派生参数是没法推导的，只能靠显式传入

// 模板类调用优先级
// 对主版本模板类、全特化类、偏特化类的调用优先级从高到低进行排序是：
// 全特化类>偏特化类>主版本模板类。
// 这样的优先级顺序对性能也是最好的。

// 但是模板特化并不只是为了性能优化，更多是为了让模板函数能够正常工作

#include <vector>
#include <iostream>
#include <cstring>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////

// 模板参数和函数参数是不同的东西，用于实例化模板的参数是模板参数，用于调用函数的参数是函数参数。
// 直观上来说，尖括号里的是模板参数，圆括号里的是函数参数。

template <typename T, class N>
void compare(T num1, N num2)
{
    cout << "standard function template" << endl;
    if (num1 > num2)
    {
        cout << "num1:" << num1 << " > num2:" << num2 << endl;
    }
    else
    {
        cout << "num1:" << num1 << " <= num2:" << num2 << endl;
    }
}

// 对部分模板参数进行特化(第一个参数)
template <class N>
void compare(int num1, N num2)
{
    cout << "partitial specialization" << endl;
    if (num1 > num2)
        cout << "num1:" << num1 << " > num2:" << num2 << endl;
    else
        cout << "num1:" << num1 << " <= num2:" << num2 << endl;
}

// 将模板参数特化为指针(模板参数的部分特性)
template <typename T, class N>
void compare(T *num1, N *num2)
{
    cout << "new partitial specialization" << endl;
    if (*num1 > *num2)
        cout << "num1:" << *num1 << " > num2:" << *num2 << endl;
    else
        cout << "num1:" << *num1 << " <= num2:" << *num2 << endl;
}

// 将模板参数特化为另一个模板类
template <typename T, class N>
void compare(std::vector<T> &vecLeft, std::vector<T> &vecRight)
{
    cout << "to vector partitial specialization" << endl;
    if (vecLeft.size() > vecRight.size())
        cout << "vecLeft.size()" << vecLeft.size() << " > vecRight.size():" << vecRight.size() << endl;
    else
        cout << "vecLeft.size()" << vecLeft.size() << " <= vecRight.size():" << vecRight.size() << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// show是一个模板函数
template <typename T>
void show(T t)
{
    std::cout << t << std::endl;
}

void DemoA()
{
    int a = 5;
    // 先来看一下[1]位置的调用，由于我们传入了参数a，编译器就会根据a的类型来推导模板参数T。
    // 在模板函数show的声明处，参数列表是(T t)，所以T会根据a的类型来推导。
    // 那么问题来了，这里到底会推导出int还是int &还是const int还是const int & ？
    // 答案也很简单，模板参数的自动推导是完全按照auto的推导原则进行的。
    show(a); // [1]

    // [2]位置的调用，我们传入的是一个常量5，照理说
    // int、const int、const int &、int &&、const int &&都可以匹配，
    // 但根据auto的推导原则，仅仅保留「最简类型」，所以仍然会推导出int。
    show(5); // [2]

    // 等同于:
    show<int>(a); // [1]
    show<int>(5); // [2]
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 推导的原则与auto相同。那么同样地，也就支持和*、&、&&、const的组合，下面给出几个实例
template <typename T>
void f1(T &t) { std::cout << "f1" << std::endl; }

template <typename T>
void f2(const T &t) { std::cout << "f2" << std::endl; }

template <typename T>
void f3(T *p) { std::cout << "f3" << std::endl; }

void DemoB()
{
    // f1(5); // 会报错，因为会推导出f1<int>，从而t的类型是int &，不能绑定常量
    int a = 1;
    f1(a); // f1<int>，t的类型是int &
    f2(a); // f2<int>，t的类型是const int &

    // f3(a); // 会报错，因为会推导出f3<int>，此时t的类型是int *，int不能隐式转换为int *
    f3(&a); // f3<int>， t的类型是int *
}
// 当T &&匹配到可变值（也就是C++11里定义的「左值」）的时候，T会推导出左值引用，再根据引用折叠原则，最终实例化为左值引用
// 当T &&匹配到不可变值（也就是C++11里定义的「右值」）的时候，T会推导出基本类型，最终实例化为右值引用

// 即便传入的本身是一个右值引用（比如上面的std::move(a)），T依然会推导为int而并不是int &&。
// 只不过实例化后的函数参数t的类型会变成int &&。

// https://blog.csdn.net/fl2011sx/article/details/128200663

////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
void add(T &t1, const T &t2)
{
    t1 += t2;
}

template <> // 模板特化也要用模板前缀，但由于已经特化了，所以参数为空
// 特化要指定模板参数，模板体中也要使用具体的类型
void add<std::string>(std::string &t1, const std::string &t2)
{
    t1 += t2;
}

void DemoC()
{
    int a = 1, b = 3;
    add(a, b); // add<int>是通过通用模板生成的，因此本质是a += b，符合预期

    std::string c1 = "abc";
    std::string c2 = "123";
    add(c1, c2); // add<char*>有定义特化，所以直接调用特化函数，因此本质是strcat(c1, c2)，符合预期
}

////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // 调用非特化版本 compare<int,int>(int num1, int num2)
    compare<int, int>(30, 31);

    // 调用偏特化版本 compare<char>(int num1, char num2)
    compare(30, '1');

    int a = 30;
    char c = '1';

    // 调用偏特化版本 compare<int,char>(int* num1, char* num2)
    compare(&a, &c);

    vector<int> vecLeft{0};
    vector<int> vecRight{1, 2, 3};

    // 调用偏特化版本 compare<int,char>(int* num1, char* num2)
    compare<int, int>(vecLeft, vecRight);

    return 0;
}
