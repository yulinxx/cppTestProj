// C++17 新特性介绍_c++17新特性_langshanglibie的博客-CSDN博客
// https://blog.csdn.net/langshanglibie/article/details/127074202

#include <iostream>
#include <string>
#include <map>
#include <any>

struct MyStruct
{
    int i = 0;
    std::string s;
};

MyStruct ms;

// 声明用两个变量,直接绑定两个成员:
// 这里u和v的声明方式被称为结构化绑定.
// 结构化绑定对于返回结构体或数组的函数尤其有用.
auto [u, v] = ms;

// 例如,有一个返回一个结构体的函数:
// 可以直接把返回值的两个数据成员赋值给两个局部变量:
MyStruct getStruct()
{
    return MyStruct{ 42, "hello" };
}

// 在这里,id 和val 分别绑定到返回的结构体中名为i 和s 的成员.
// 它们的类型分别是int 和std::string.
auto [id, val] = getStruct(); // id 和val 分别对应返回结构体的i 和 s 成员

// [[nodiscard]]可以鼓励编译器在某个函数的返回值未被使用时给出警告.
[[nodiscard]] char* fooNodiscard()
{
    char* p = new char[100];
    return p;
};

int main()
{
    // --- 结构化绑定 1
    // 结构化绑定的两点优势
    // 一是无需再用成员运算符间接访问,而是可以直接访问成员,简化代码
    // 二是可以将值绑定到一个能体现语义的变量名上,增强代码的可读性
    ms.i = 999;
    ms.s = "axax";
    auto [u, v] = ms;
    std::cout << "u" << u << " v" << v << std::endl;

    // --- 结构化绑定 2
    std::map<int, std::string> mapTest;
    mapTest.insert({ 1, "abc" });
    mapTest.insert({ 2, "bcd" });
    mapTest[3] = "cde";

    for (const auto& [k, v] : mapTest)
    {
        std::cout << " Map key:" << k << " value:" << v << std::endl;
    }

    auto funcTest = [](int n) {
        return (n % 3);
        };

    if (int n = funcTest(32); n == 2)
    {
        std::cout << "带初始化的if语句" << std::endl;
        // std::cout<<"带if语句中初始化了一个变量n,这个变量仅在整个if语句中可访问的."<<std::endl;
    }

    // --- 聚合体扩展
    // C++17 之前就有一种聚合体专有的始化方法,叫做聚合体初始化.
    // 这是从 C 语言引入的初始化方式,是用大括号括起来的一组值来初始化类:
    struct Data
    {
        std::string name;
        double value;
    };
    Data x = { "test1", 6.778 };

    // C++17 对聚合体的概念进行了扩展,聚合体可以拥有基类了.
    // 也就是说像下面这样的派生类,也可以使用聚合体初始化:
    struct MoreData : Data
    {
        bool done;
    };
    MoreData y{ {"test1", 6.778}, false }; // {"test1", 6.778} 用来初始化基类

    // --- 新属性
    auto foo = [](int error) {
        switch (error)
        {
        case 1:
            // [[fallthrough]]可以避免编译器在switch语句中,当某一个标签缺少break语句时发出警告.
            [[fallthrough]];

        case 2:
            std::cout << "Error happened";
            break;

        default:
            std::cout << "OK";
            break;
        }
        };

    foo(1);

    // warning C4834: 放弃具有 "nodiscard" 属性的函数的返回值
    fooNodiscard(); // 编译器发出警告

    std::cout << std::endl;
    std::cout << u8"abc" << std::endl;
    std::cout << "def" << std::endl;

    // std::any  std::any可以存储任何类型的单个值.
    std::any a;            // a 为空
    std::any b = 4.3;      // b 有类型为 double 的值 4.3
    a = 42;                // a 有类型为 int 的值 42
    b = std::string{ "hi" }; // b 有类型为 std::string 的值 "hi"

    if (a.type() == typeid(std::string))
    {
        std::string s = std::any_cast<std::string>(a);
        // useString(s);
    }
    else if (a.type() == typeid(int))
    {
        // useInt(std::any_cast<int>(a));
    }
    return 0;
}