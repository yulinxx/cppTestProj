// std::string_view 是 C++17 中引入的标准库类型，用于表示字符串的只读视图。
// 它提供了一种轻量级的方式来访问和操作字符串，而无需进行复制或分配新的内存。

// std::string_view 并不拥有自己的字符串数据，它只是一个指向现有字符串数据的指针和长度的组合。
// 因此，std::string_view 是一种非常高效的字符串处理工具，特别适用于在不修改字符串的情况下执行字符串操作。

// 以下是一些std::string_view 的主要特点和用法：

// 1. 非拥有性：std::string_view 不拥有字符串数据，只是对现有字符串的引用。它适用于临时的、只读的字符串操作。
// 2. 轻量级：std::string_view 的大小很小，通常只是一个指针和一个长度。

// 3. 字符串访问：可以使用std::string_view 的成员函数（如size()、empty()）来获取字符串的长度和判断字符串是否为空。
// 4. 字符串操作：std::string_view 支持多种字符串操作，如子字符串提取、比较、查找等。可以使用它的成员函数和标准库算法来执行这些操作。

// 5. 字符串传递：std::string_view 可以作为函数参数来传递字符串视图，避免不必要的字符串复制和分配。

// 以下是一个简单示例，展示了如何使用std::string_view：

#include <iostream>
#include <string_view>

void processString(std::string_view str)
{
    std::cout << "Len:" << str.size() << std::endl;
    std::cout << "Is Empty:" << std::boolalpha << str.empty() << std::endl;
    std::cout << "5 chars:" << str.substr(0, 5) << std::endl;
}

int main()
{
    std::string str = "Hello, world!";
    std::string_view strView(str);

    processString(strView);

    return 0;
}

// 在这个示例中，我们首先创建了一个std::string 类型的字符串str，
// 然后通过将其传递给std::string_view 构造函数来创建了一个字符串视图strView。
// 我们将strView 传递给processString 函数，该函数接受一个std::string_view 参数并对其进行操作。

//std::string_view 可以提供对str 字符串的访问和操作，而无需进行复制或分配新的内存。