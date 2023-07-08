// std::variant 是 C++17 中引入的标准库类型，用于表示可以存储多个不同类型值的变体（variant）。
// 它类似于联合（union），但提供了更丰富的类型安全性和语法支持。

// std::variant 允许在同一个变量中存储多个不同的类型，但每次只能存储其中之一。
// 这使得 std::variant 成为一种强类型的联合，可以更好地保证类型安全，避免了使用传统联合时的类型转换问题。

// 以下是一些 std::variant 的主要特点和用法：

// 1. 多类型支持：std::variant 可以存储多个不同类型的值。
// 可以通过模板参数指定可接受的类型列表，或使用 std::variant 的别名 std::variant<T1, T2, ...>。

// 2. 类型安全：std::variant 在编译时进行类型检查，确保只存储指定类型之一的值。
// 可以使用 std::holds_alternative 函数来检查 std::variant 是否包含特定类型的值。

// 3. 值获取：可以使用 std::get 函数从 std::variant 中获取特定类型的值。
// 需要注意，获取值之前应该先确认 std::variant 中确实包含了该类型的值。

// 4. 值访问：可以使用 std::visit 函数对 std::variant 中的值进行访问，
// 提供一组访问函数对象（Visitor）或 Lambda 表达式，以根据当前存储的值类型执行相应的操作。


#include <iostream>
#include <variant>
#include <string>

int main()
{
    std::variant<int, double, std::string> myVariant;

    myVariant = 42; // 存储 int 类型的值
    std::cout << std::get<int>(myVariant) << std::endl;

    myVariant = 3.14; // 存储 double 类型的值
    std::cout << std::get<double>(myVariant) << std::endl;

    myVariant = "Hello, world!"; // 存储 std::string 类型的值
    std::cout << std::get<std::string>(myVariant) << std::endl;

    std::variant<int, double, std::string> myVariant;                    // 默认构造，无值
    std::variant<int, double, std::string> myVariantWithValue(42);       // 使用值初始化
    std::variant<int, double, std::string> myVariantWithString("Hello"); // 使用字符串初始化

    if (std::holds_alternative<int>(myVariant))
    {
        // 值是 int 类型
    }
    else if (std::holds_alternative<double>(myVariant))
    {
        // 值是 double 类型
    }
    else if (std::holds_alternative<std::string>(myVariant))
    {
        // 值是 std::string 类型
    }

    // 使用 holds_alternative 函数来检查当前存储的值的类型，并使用 get 函数来获取特定类型的值
    // std::holds_alternative 是 C++17 中引入的标准库函数，用于检查 std::variant 中存储的值是否与指定的类型匹配。
    if (std::holds_alternative<int>(myVariant))
    {
        int value = std::get<int>(myVariant); // 获取 int 类型的值，如果类型不匹配则抛出 std::bad_variant_access 异常
    }
    else if (std::holds_alternative<double>(myVariant))
    {
        double value = std::get<double>(myVariant); // 获取 double 类型的值，如果类型不匹配则抛出 std::bad_variant_access 异常
    }
    else if (std::holds_alternative<std::string>(myVariant))
    {
        std::string value = std::get<std::string>(myVariant); // 获取 std::string 类型的值，如果类型不匹配则抛出 std::bad_variant_access 异常
    }

    return 0;
}


// 在这个示例中，我们创建了一个 std::variant 类型的变量 myVariant，
// 它可以存储 int、double 和 std::string 三种类型的值。
// 我们依次存储了不同类型的值，并使用 std::get 函数来获取存储的值并打印到标准输出。

// std::variant 提供了一种灵活的方式来处理多类型值的情况，使得代码更具表达力和可维护性。
// 需要注意的是，使用 std::variant 时应格外注意类型转换和异常处理，以避免出现不正确的类型访问或未处理的异常。

