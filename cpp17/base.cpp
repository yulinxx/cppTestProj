// C++17 新特性介绍_c++17新特性_langshanglibie的博客-CSDN博客
// https://blog.csdn.net/langshanglibie/article/details/127074202

#include <iostream>
#include <string>

struct MyStruct
{
    int i = 0;
    std::string s;
};

MyStruct ms;

// 声明用两个变量，直接绑定两个成员：
// 这里u和v的声明方式被称为结构化绑定。
// 结构化绑定对于返回结构体或数组的函数尤其有用。
auto [u, v] = ms;

// 例如，有一个返回一个结构体的函数：
// 可以直接把返回值的两个数据成员赋值给两个局部变量：
MyStruct getStruct()
{
    return MyStruct{42, "hello"};
}

// 在这里，id 和val 分别绑定到返回的结构体中名为i 和s 的成员。
// 它们的类型分别是int 和std::string。
auto [id, val] = getStruct(); // id 和val 分别对应返回结构体的i 和 s 成员

int main()
{
    // 结构化绑定的两点优势
    // 一是无需再用成员运算符间接访问，而是可以直接访问成员，简化代码
    // 二是可以将值绑定到一个能体现语义的变量名上，增强代码的可读性
    ms.i = 999;
    ms.s = "axax";
    auto [u, v] = ms;
    std::cout<<"u"<<u<<" v"<<v<<std::endl;
    return 0;
}