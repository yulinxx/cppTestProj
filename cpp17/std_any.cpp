// std::any 是 C++17 中引入的标准库类型，它允许在一个单一的变量中存储任何类型的值，
// 类似于动态语言中的通用类型。下面是一个简单的示例，展示了如何使用 std::any：

#include <iostream>
#include <any>

int main() 
{
    std::any anyValue;

    // 存储整数类型
    anyValue = 10;
    if (anyValue.has_value()) 
    {
        std::cout << "存储的值是整数: " << std::any_cast<int>(anyValue) << std::endl;
    }

    // 存储字符串类型
    anyValue = std::string("Hello, world!");
    if (anyValue.has_value())
    {
        std::cout << "存储的值是字符串: " << std::any_cast<std::string>(anyValue) << std::endl;
    }

    // 存储自定义类型
    struct MyStruct {
        int data;
    };

    MyStruct myObj{42};
    anyValue = myObj;
    
    if (anyValue.has_value())
    {
        MyStruct retrievedObj = std::any_cast<MyStruct>(anyValue);
        std::cout << "存储的自定义类型的值是: " << retrievedObj.data << std::endl;
    }

    // 清除值
    anyValue.reset();
    if (!anyValue.has_value())
    {
        std::cout << "值已被清除" << std::endl;
    }

    return 0;
}


// 在这个示例中，我们首先创建了一个 std::any 类型的变量 value。
// 然后，我们存储了一个整数、一个字符串和一个自定义的结构体对象。
// 我们使用 has_value() 函数来检查 value 是否包含一个值，
// 然后使用 std::any_cast 来获取存储的具体类型，并进行相应的操作。
// 最后，我们通过调用 reset() 来清除 value 中的值。

// 需要注意的是，使用 std::any 时需要小心处理类型转换错误和空值检查，
// 否则可能会引发 std::bad_any_cast 异常。
