// std::optional 是 C++17 中引入的标准库类型,用于表示可选值的存在或缺失状态.
// 它提供了一种机制来处理可能没有值的情况,避免了使用特殊值(如空指针或特殊常量)来表示缺失值的问题.

// std::optional 是一种类型安全的包装器,可以将一个值或没有值的状态包裹在其中.它类似于容器,但只能容纳一个元素.

// 以下是一些 std::optional 的主要特点和用法:

// 1. 值的存在状态:std::optional 可以表示值的存在或缺失状态.当 std::optional 包含值时,它处于已初始化状态;
// 当 std::optional 不包含值时,它处于未初始化状态.

// 2. 安全获取值:可以使用 std::optional 的成员函数 value() 或 operator*() 来获取包含的值.
// 在获取值之前,应该使用 has_value() 成员函数检查 std::optional 是否包含值,以避免访问未初始化的状态.

// 3. 默认值和替代值:可以使用 std::optional 的 value_or() 成员函数来获取包含的值,
// 如果 std::optional 不包含值,则返回提供的默认值或替代值.

// 4. 值的重置和清除:可以使用 reset() 成员函数来将 std::optional 设置为未初始化状态,
// 或使用 std::nullopt 或 std::nullopt_t 值来清除 std::optional 的值.

// 以下是一个简单示例,展示了如何使用 std::optional:

#include <iostream>
#include <optional>

std::optional<int> divide(int a, int b)
{
    if (b != 0)
    {
        return a / b;
    }
    else
    {
        return std::nullopt; // 表示缺失值
    }
}

int main()
{
    std::optional<int> result = divide(10, 2);

    if (result.has_value()) // 包含值的情况
    {
        std::cout << "结果: " << result.value() << std::endl;
    }
    else    // 无值的情况
    {
        std::cout << "除法错误" << std::endl;
    }

    std::optional<int> myOptional;                    // 默认构造,无值
    std::optional<int> myOptionalWithValue(42);       // 使用值初始化
    std::optional<int> myOptionalNull = std::nullopt; // 显式设置为无值

    return 0;
}

// 在这个示例中,我们定义了一个 divide() 函数,用于执行整数除法.如果除数不为零,则函数返回结果的 std::optional 包装值;
// 如果除数为零,则返回 std::nullopt 表示缺失值.
// 在 main() 函数中,我们使用 has_value() 和 value() 成员函数来检查并获取包装的结果值.

// std::optional 提供了一种更安全、更语义化的方式来表示可能缺失的值,减少了在代码中使用特殊值或空指针的需要.
// 它使得代码更易于理解、维护和调试.