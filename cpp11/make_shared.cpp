// std::make_shared 是 C++11 标准库中的函数模板，用于创建和初始化 std::shared_ptr 对象。
// 它提供了一种更方便、更安全的方式来管理动态分配的对象的生命周期。

// std::make_shared 接受可变数量的参数，并在动态内存中分配一个对象，并返回一个指向该对象的 std::shared_ptr 智能指针。
// 它将对象的构造函数参数传递给对象的构造函数，自动进行内存管理，确保在不再需要该对象时正确地释放内存。

// 以下是 std::make_shared 的示例用法：

#include <memory>
#include <iostream>

class MyClass
{
public:
    MyClass(int value) : data(value)
    {
        std::cout << "MyClass constructor" << std::endl;
    }

    ~MyClass()
    {
        std::cout << "MyClass destructor" << std::endl;
    }

private:
    int data;
};

int main()
{
    std::shared_ptr<MyClass> ptr = std::make_shared<MyClass>(42);
    // 使用 std::shared_ptr 管理动态分配的对象

    return 0;
}

// 在这个示例中，我们定义了一个简单的类 MyClass，它接受一个 int 类型的参数。
// 在 main() 函数中，我们使用 std::make_shared 创建了一个 std::shared_ptr 对象，
// 用于管理通过 MyClass 构造函数动态分配的对象。
// std::make_shared 会调用 MyClass 的构造函数并传递给定的参数，自动分配内存，
// 并返回一个指向对象的 std::shared_ptr 智能指针。

// std::make_shared 具有以下优点：
// 1. 通过一次内存分配，同时分配对象和控制块（用于管理引用计数等信息），提高了性能和内存利用率。
// 2. 通过使用 std::make_shared，避免了直接使用 new 运算符创建对象和手动释放内存的繁琐工作。
// 智能指针会自动处理对象的生命周期，确保在不再需要时正确释放内存。

// 3. 使用 std::make_shared 创建的 std::shared_ptr 对象是强类型的，
// 可以自动执行引用计数管理，并且可以方便地进行拷贝、赋值和比较。

// 需要注意的是，std::make_shared 在动态内存中分配对象时使用了完美转发，
// 可以接受可变数量和类型的参数，并将它们传递给对象的构造函数。
// 这使得创建对象更加灵活，可以适应不同的构造函数签名。

// 使用 std::make_shared 可以提供更安全、更便捷的对象管理方式，避免手动处理内存分配和释放的复杂性。
// 它是 C++ 中动态对象管理的一种推荐方式。
