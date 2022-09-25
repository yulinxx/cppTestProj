#include <iostream>
#include <memory>

struct DataA { int *data; };
struct Data { int a; int b; };

int main()
{
    // 构造函数
    {
        std::shared_ptr<int> p1;
        std::shared_ptr<int> p2(nullptr);
        std::shared_ptr<int> p3(new int);
        std::shared_ptr<int> p4(new int, std::default_delete<int>());
        std::shared_ptr<int> p5( new int, [](int *p)
            { delete p; }, std::allocator<int>());
            
        std::shared_ptr<int> p6(p5);
        std::shared_ptr<int> p7(std::move(p6));
        std::shared_ptr<int> p8(std::unique_ptr<int>(new int));
        std::shared_ptr<DataA> obj(new DataA);
        std::shared_ptr<int> p9(obj, obj->data);

        std::cout << "use_count:\n";
        std::cout << "p1: " << p1.use_count() << '\n';
        std::cout << "p2: " << p2.use_count() << '\n';
        std::cout << "p3: " << p3.use_count() << '\n';
        std::cout << "p4: " << p4.use_count() << '\n';
        std::cout << "p5: " << p5.use_count() << '\n';
        std::cout << "p6: " << p6.use_count() << '\n';
        std::cout << "p7: " << p7.use_count() << '\n';
        std::cout << "p8: " << p8.use_count() << '\n';
        std::cout << "p9: " << p9.use_count() << '\n';
    }

    // 析构函数
    {
        auto deleter = [](int *p)
        {
            std::cout << "[deleter called]\n";
            delete p;
        };

        std::shared_ptr<int> foo(new int, deleter);

        std::cout << "use_count: " << foo.use_count() << '\n';
    }

    // 赋值运算“=”
    {
        std::shared_ptr<int> foo;
        std::shared_ptr<int> bar (new int(10));

        foo = bar;                          // copy

        bar = std::make_shared<int> (20);   // move

        std::unique_ptr<int> unique (new int(30));
        foo = std::move(unique);            // move from unique_ptr

        std::cout << "*foo: " << *foo << '\n';
        std::cout << "*bar: " << *bar << '\n';
    }

    // swap函数
    // void swap (shared_ptr& x) noexcept; 
    // 参数x: 另一个相同类型的 shared_ptr 对象（即，具有相同的类模板参数 T）。
    // 作用是将 shared_ptr 对象的内容与 x 的内容交换，在它们之间转移任何托管对象的所有权，而不会破坏或改变两者的使用计数。
    {
        std::shared_ptr<int> foo(new int(10));
        std::shared_ptr<int> bar(new int(20));

        foo.swap(bar);

        std::cout << "*foo: " << *foo << '\n';
        std::cout << "*bar: " << *bar << '\n';
    }

    // reset函数
    // 重置shared_ptr，对于声明1) 对象变为空（如同默认构造）。
    // 在所有其他情况下，shared_ptr 以使用计数为 1 获取 p 的所有权，并且 - 可选地 - 使用 del 和/或 alloc 作为删除器 和分配器。
    // 另外，调用这个函数有同样的副作用，就像在它的值改变之前调用了shared_ptr 的析构函数一样
    // （包括删除托管对象，如果这个shared_ptr 是唯一的）。
    {
        std::shared_ptr<int> sp; // empty

        sp.reset(new int); // takes ownership of pointer
        *sp = 10;
        std::cout << *sp << '\n';

        sp.reset(new int); // deletes managed object, acquires new pointer
        *sp = 20;
        std::cout << *sp << '\n';

        sp.reset(); // deletes managed object
    }

    // get函数
    // 函数声明：element_type* get() const noexcept; get()返回存储的指针。
    // 存储的指针指向shared_ptr对象解引用的对象，一般与其拥有的指针相同。
    // 存储的指针（即这个函数返回的指针）可能不是拥有的指针（即对象销毁时删除的指针）
    // 如果 shared_ptr 对象是别名（即，别名构造的对象及其副本）
    {
        int *p = new int(10);
        std::shared_ptr<int> a(p);

        if (a.get() == p)
            std::cout << "a and p point to the same location\n";

        // three ways of accessing the same address:
        std::cout << *a.get() << "\n";
        std::cout << *a << "\n";
        std::cout << *p << "\n";
    }

    // 取对象运算“*”
    // 函数声明：element_type& operator*() const noexcept;
    // 取消引用对象。返回对存储指针指向的对象的引用。等价于：*get()。
    // 如果shared_ptr的模板参数为void，则该成员函数是否定义取决于平台和编译器，
    // 以及它的返回类型 在这种情况下。
    {
        std::shared_ptr<int> foo(new int);
        std::shared_ptr<int> bar(new int(100));

        *foo = *bar * 2;

        std::cout << "foo: " << *foo << '\n';
        std::cout << "bar: " << *bar << '\n';
    }

    // “->”操作符
    // element_type* operator->() const noexcept; 取消引用对象成员。
    // 返回一个指向存储指针指向的对象的指针，以便访问其成员之一。
    // 如果存储的指针是空指针，则不应调用该成员函数，它返回与 get() 相同的值。
    {
        std::shared_ptr<Data> foo;
        std::shared_ptr<Data> bar(new Data);

        foo = bar;

        foo->a = 10;
        bar->b = 20;

        if (foo)
            std::cout << "foo: " << foo->a << ' ' << foo->b << '\n';
        if (bar)
            std::cout << "bar: " << bar->a << ' ' << bar->b << '\n';
    }

    // unique函数
    {
        std::shared_ptr<int> foo;
        std::shared_ptr<int> bar(new int);

        std::cout << "foo unique?\n"
                  << std::boolalpha;

        std::cout << "1: " << foo.unique() << '\n'; // false (empty)

        foo = bar;
        std::cout << "2: " << foo.unique() << '\n'; // false (shared with bar)

        bar = nullptr;
        std::cout << "3: " << foo.unique() << '\n'; // true
    }

    // “bool”操作
    {
        std::shared_ptr<int> foo;
        std::shared_ptr<int> bar(new int(34));

        if (foo)
            std::cout << "foo points to " << *foo << '\n';
        else
            std::cout << "foo is null\n";

        if (bar)
            std::cout << "bar points to " << *bar << '\n';
        else
            std::cout << "bar is null\n";
    }

    // owner_before函数
    {
        int *p = new int(10);

        std::shared_ptr<int> a(new int(20));
        std::shared_ptr<int> b(a, p); // alias constructor

        std::cout << "comparing a and b...\n"
                  << std::boolalpha;
        std::cout << "value-based: " << (!(a < b) && !(b < a)) << '\n';
        std::cout << "owner-based: " << (!a.owner_before(b) && !b.owner_before(a)) << '\n';

        delete p;
    }

    
    return 0;
}