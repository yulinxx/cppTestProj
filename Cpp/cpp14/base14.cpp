#include <iostream>
#include <tuple>
#include <string>
#include <memory>

// 泛型 lambda 函数遵循模板参数推导的规则。
struct unnamed_lambda
{
    template <typename T, typename U>
    auto operator()(T x, U y) const
    {
        return x + y;
    }
};

int main()
{
    // C++14提供了在lambda式的形参声明中使用auto的能力
    // 泛型 lambda：C++14 中的 lambda 表达式可以使用模板参数，使其成为一个泛型函数对象。
    // 这样可以实现更加通用的操作。例如：
    auto print = [](auto value) {
        std::cout << value << std::endl;
        };

    print(10);      // 输出：10
    print("Hello"); // 输出：Hello

    // 函数返回类型推导
    // lambda表达式可以使用auto形参和返回类型，但普通函数不能使用auto形参。
    // 虽然普通函数不能使用auto形参，但可以使用返回类型推导。
    // 在C++ 11中我们可以使用后置返回类型推导来完成这件事情：
    auto add11a = [](int x, int y)->decltype(x + y) {
        return x + y;
        };

    // 而在C++ 14中，我们可以省略decltype，编译器直接由return语句的值推导出返回类型：
    auto add14b = [](int x, int y) {
        return x + y;
        };

    // 如果函数实现中含有多个 return 语句，这些表达式必须可以推断为相同的类型
    auto PI1 = [](bool fract) {
        if (fract)
            return 3.1416;
        else
            return 3.0;
        // return 3; // 错误：推导出来的int与之前推导出来的double冲突
        };

    // 对于含有递归调用的函数，递归调用必须在第一个return之后
    // auto Correct1 = [](int i)
    // {
    //     if (i == 1)
    //         return i; // 返回类型被推断为 int
    //     else
    //         return Correct1(i - 1) + i; // 正确，可以调用
    // };

    auto Wrong1 = [](int i) {
        if (i != 1)
            return i;
        // return Wrong1(i - 1) + i; // 不能调用，之前没有 return 语句
        else
            return i; // 返回类型被推断为 int
        };

    // 二进制字面值
    // 二进制字面值可以使用0b或者0B开头来表示
    int iA = 0b0100010001; // 273
    int iB = 0B0100010001; // 273，大写B

    // 数字分位符
    // C++14 引入单引号（’）作为数字分位符号，使得数值型的字面量可以具有更好的可读性

    auto integer_literal = 100'0000;
    auto floating_point_literal = 1.797'693'134'862'315'7E+308;
    auto binary_literal = 0b0100'1100'0110;
    auto silly_example = 1'0'0'000'00;

    // 通过类型寻址多元组
    // C++11 引入的 std::tuple 类型允许不同类型的值的聚合体用编译期整型常数索引。
    // C++14还允许使用类型代替常数索引，从多元组中获取对象。
    // 若多元组含有多于一个这个类型的对象，将会产生一个编译错误：

    std::tuple<std::string, std::string, int> t("foo", "bar", 7);
    int i = std::get<int>(t);                 // i == 7
    int j = std::get<2>(t);                   // Same as before: j == 7

    // 多个相同类型的元素存在于元组中，会导致编译错误
    // std::string s = std::get<std::string>(t); // Compiler error due to ambiguity
    std::string s = std::get<0>(t); // No ambiguity: s == "foo"

    // make_unique
    // std::make_unique 可以像 std::make_shared 一样使用，用于产生 std::unique_ptr 对象

    std::unique_ptr<int> ptr = std::make_unique<int>(42);
    return 0;
}