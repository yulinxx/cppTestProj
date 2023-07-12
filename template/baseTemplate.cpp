#include <string>
// https://blog.csdn.net/c1sdn19/article/details/123669618

// 只有编译期能够确定的量，才能用来实例化模板。o

// 模板特化（template specialization）不同于模板的实例化，模板参数在某种特定类型下的具体实现称为模板特化。
// 模板特化有时也称之为模板的具体化，分别有函数模板特化和类模板特化。

template <typename NumType>
class Obj
{
};
// template <typename NumType>
// class Obj1<NumType, 9>
// {
// public:
//     NumType array[]
// };

// 偏特化模板
template <typename NumType, int count = 9>
class Obj2
{
public:
    NumType array[count];
};

template <typename T, typename N, int d = 9>
class Obj3
{
public:
    T arrayA[3];
    N arrayB[3];
};

// template <typename T,  size_t size>
// class Array
// {
// public:
//     Array() {}
//     T &at(int n) {return data_[n];}
//     size_t size() const {}

// private:
//     T data_[size];
// };

// 函数类型模板参数
template <void func()>
void f()
{
    func();
}

// 普通函数
void f1() {}

class Test
{
public:
    void f2() {}        // 成员函数
    static void f3() {} // 静态成员函数
};

// 变参模板 https://blog.csdn.net/fl2011sx/article/details/128077440
template <typename... Args>
auto sum(Args... args)
{
    return (... + args);
}

int main()
{
    Obj<int> obj;
    Obj2<int, 2> obj2;
    Obj3<int, float, 3> obj3;

    // Obj<std::string, 2> objC();
    // Obj<int, 4> objD;

    int nA = 43, nB = 84;
    // test.Set(nA, nB);

    // Obj<int, 2> testB(test);


    // Array<int, 5> arr; // 实例化，并创建对象
    // arr.at(1) = 6;

    // 函数类型模板参数 使用
    void (*pf1)() = &f1;           // 局部变量
    constexpr void (*pf2)() = &f1; // 常量表达式

    f<f1>();        // OK
    f<&f1>();       // OK
    f<Test::f3>();  // OK
    f<&Test::f3>(); // OK
    f<pf2>();       // OK

    // 变参模板 的使用
    auto a = sum(1, 2, 3, 4);                       // 10
    auto b = sum(1.5, 2, 3l);                       // 6.5
    auto c = sum(std::string("abc"), "def", "111"); // std::string("abcdef111")
    auto d = sum(2);                                // 2
    return 0;
}
