// 一般来说，c++是一种具有类型绑定和类型安全性的语言。
// 值对象声明为具有特定类型，该类型定义哪些操作是可能的以及它们的行为方式。值对象不能改变它们的类型。

// std: any是一种值类型，它能够更改其类型，同时仍然具有类型安全性。
// 也就是说，对象可以保存任意类型的值，但是它们知道当前保存的值是哪种类型。
// 在声明此类型的对象时，不需要指定可能的类型。
// 诀窍在于，对象同时拥有包含的值和使用 typeid 包含值的类型。
// 因为这个值可以有任何大小，所以可以在堆上分配内存，鼓励实现避免小对象的动态分配。
// 也就是说，如果分配一个字符串，对象将为该值分配内存并复制该字符串，同时也在内部存储分配的字符串。
// 稍后，可以执行运行时检查来确定当前值的类型，并使用any_cast<该值的类型>获取值。

// std::any 是 C++17 中引入的标准库类型，它允许在一个单一的变量中存储任何类型的值，
// 类似于动态语言中的通用类型。

// constructors	    创建一个any对象(可能调用底层类型的构造函数)
// make_any()	    创建一个any对象(传递值来初始化它)
// destructor	    销毁any对象
// =	            分配一个新值
// emplace<T>()	    分配一个类型为T的新值
// reset()	        销毁any对象的值(使对象为空)
// has_value()	    返回对象是否具有值
// type()	        返回当前类型为std::type_info对象
// any_cast<T>()	使用当前值作为类型T的值(如果其他类型除外)
// swap()	        交换两个any对象的值

// std::any没有定义比较运算符(因此，不能比较或排序对象)，没有定义hash函数，也没有定义value()成员函数。
// 由于类型只在运行时才知道，所以不能使用泛型lambdas处理与类型无关的当前值。
// 总是需要运行时函数std::any_cast<>来处理当前值。

#include <iostream>
#include <vector>
#include <any>

int main()
{
    std::any anyValue;

    // 存储整数类型
    anyValue = 10;
    if (anyValue.has_value())
    {
        std::cout << "存储的值是整数: " << std::any_cast<int>(anyValue) << std::endl;

        try
        {
            auto s = std::any_cast<float>(anyValue);
        }
        catch (std::bad_any_cast& e)
        {
            // 如果转换失败，因为对象为空或包含的类型不匹配，则抛出std::bad_any_cast
            std::cerr << "EXCEPTION: " << e.what() << '\n';
        }
    }

    // 存储字符串类型
    anyValue = std::string("Hello, world!");
    if (anyValue.has_value())
    {
        // 将std::string作为模板参数传递给std::any_cast<>，它将创建一个临时string(一个prvalue)，然后用它初始化新对象s。
        // 如果没有这样的初始化，通常最好转换为引用类型，以避免创建临时对象:
        std::cout << "存储的值是字符串: " << std::any_cast<std::string>(anyValue) << std::endl;

        std::cout << std::any_cast<const std::string&>(anyValue);
    }

    // 存储自定义类型
    struct MyStruct
    {
        int data;
    };

    MyStruct myObj{ 42 };
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
        std::cout << "Clear..." << std::endl;
    }

    std::vector<std::any> v;
    v.push_back(42);
    std::string s = "hello";
    v.push_back(s);
    for (const auto& a : v)
    {
        if (a.type() == typeid(std::string))
        {
            std::cout << "string: " << std::any_cast<const std::string&>(a) << '\n';
        }
        else if (a.type() == typeid(int))
        {
            std::cout << "int: " << std::any_cast<int>(a) << '\n';
        }
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