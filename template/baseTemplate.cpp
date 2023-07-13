#include <string>
#include <vector>
#include <iostream>

// https://blog.csdn.net/c1sdn19/article/details/123669618

// 只有编译期能够确定的量，才能用来实例化模板。o

// 模板特化（template specialization）不同于模板的实例化，模板参数在某种特定类型下的具体实现称为模板特化。
// 模板特化有时也称之为模板的具体化，分别有函数模板特化和类模板特化。

// 模板是编译期语法，因此，这里的整型数据也必须是编译期能确定的，比如说常数、常量表达式等，而不可以是动态的数据。
// 在C++20以前，只允许整数参数(本质上是整数的类型)，但从C++20起，可以支持浮点数做参数

// C++的模板并非直接可用的代码，需要经历「实例化」，而实例化的过程其实就是指定参数的过程。

// template <typename T>
// void f(const T &t) {}
// 对于那些基本类型（比如说int、char之类的）来说，是徒增了它的开销吗? 为什么

// 对于基本类型（如 int、char 等），传递参数作为 const T& 的形式可能会增加一些开销。
// 这是因为模板函数 f 会对传递的参数进行类型推断，并使用对应的类型 T 进行实例化。
// 对于基本类型，包括内置类型，这意味着会进行额外的参数拷贝或引用。

// 当使用 const T& 进行参数传递时，会发生以下过程：

// 1. 对于非引用类型的参数，会创建一个临时对象来进行拷贝构造，以便传递给模板函数。
// 2. 对于引用类型的参数，不会发生额外的拷贝构造，但会增加对临时对象的引用。

// 因此，对于基本类型，可能会增加一些开销，例如拷贝构造函数的调用或引用计数的增加。
// 这个开销通常是微不足道的，对于大多数情况下的基本类型，不会对性能产生显著影响。

// 然而，对于某些特殊情况，比如需要频繁调用的高性能代码段，可以考虑使用非模板函数或使用传值方式（const T）进行参数传递，以避免不必要的开销。

// 总结起来，对于基本类型，使用模板函数和传递 const T& 形式的参数可能会引入一些微小的开销，但在大多数情况下这种开销是可以忽略的。
// 如果性能是关键考虑因素，可以选择其他方式来处理基本类型参数

////////////////////////////////////////////////////////////////////////////////////////////////
template <typename NumType>
class Obj
{
};

template <typename NumType, int count = 9>
class Obj1
{
public:
  NumType array[count];
};

// 偏特化模板
template <typename NumType, int count = 9>
class Obj2
{
public:
  NumType array[count];
};

////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t sz>
class Array
{
public:
  Array() {}
  T &at(int n) { return data_[n]; }
  size_t size() const { return sz; }

private:
  T data_[sz];
};

// 函数类型模板参数
template <void func()>
void f()
{
  func();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 变参模板 https://blog.csdn.net/fl2011sx/article/details/128077440
template <typename... Args>
auto sum(Args... args)
{
  return (... + args);
}

// 整数参数的模板
template <int N>
struct TestB
{
};

void Demo()
{
  TestB<5> t1; // 常数OK

  constexpr int a = 5;
  TestB<a> t2; // 常量表达式OK

  const int b = 6;
  // Test<b> t3; // ERR，b是只读变量，不是常量

  TestB<a * 3> t4; // 常数运算OK

  std::vector<int> ve{1, 2, 3};
  // TestB<ve.size()> t5; // ERR，size是运行时数据
  // TestB<ve[1]> t6; // ERR，ve的成员是运行时数据

  int arr1[]{1, 2, 3};
  // TestB<arr1[0]> t6; // ERR，arr1的成员是运行时数据

  constexpr int arr2[]{2, 4, 6};
  TestB<arr2[1]> t7; // 常量表达式修饰的普通数组成员OK
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 「模板类型的模板参数」，其实就是嵌套模板的意思，把「某一种类型的模板」作为一个参数传给另一个模板
// 模板类型的模板参数
template <template <typename, typename> typename Tem>
void f()
{
  Tem<int, std::string> te;
  te.show();
}

// 符合条件的模板类
template <typename T1, typename T2>
struct Test1
{
  void show() { std::cout << 1 << std::endl; }
};

template <typename T1, typename T2>
struct Test2
{
  void show() { std::cout << 2 << std::endl; }
};

// 不符合条件的模板类
template <int N>
struct Test3
{
  void show() { std::cout << 3 << std::endl; }
};

void Demo2()
{
  f<Test1>(); // 注意这里，要传模板，而不是实例化后的模板
  // f<Test<int, int>>(); // ERR，模板参数类型不匹配

  f<Test2>(); // OK
  // f<Test3>(); // ERR，类型不匹配
}

namespace Simple
{
  // 一个简单的函数模板例子
  template <typename T>
  T get_max(T a, T b)
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    std::cout << a << " " << b << std::endl;
    auto result = a > b ? a : b;
    return result;
  }

  // 提供特化版本，处理某些特定类型
  template <>
  float get_max(float a, float b)
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    std::cout << "float : " << a << " " << b << std::endl;
    auto result = a > b ? a : b;
    return result;
  }

  // 提供偏特化版本，解决传递指针的问题
  template <typename T>
  T get_max(T *a, T *b)
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    std::cout << *a << " " << *b << std::endl;
    auto result = *a > *b ? *a : *b;
    return result;
  }

  // 默认值模板
  template <typename T = std::string>
  T get_max(std::string a, std::string b) // 默认值这里的参数类型需要直接指定
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    std::cout << "STRING : " << a << " " << b << std::endl;
    return a;
  }

  // 尾置返回类型
  template <typename T1, typename T2>
  auto get_max(T1 a, T2 b) -> decltype(b < a ? a : b)
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    std::cout << a << " " << b << std::endl;
    return a > b ? a : b;
  }

  void Demo()
  {
    int a = 8, b = 3;
    auto c = a + b;    // 运行时需要实际执行a+b，哪怕编译时就能推导出类型
    decltype(a + b) d; // 编译期类型推导
    // auto c;  // error // 不可以用auto c; 直接声明变量，必须同时初始化。
  }
}

namespace Decay
{
  // decay   英 [dɪˈkeɪ]美 [dɪˈkeɪ]
  // v. （建筑、地方等）破败，衰落；（观念、影响力等）衰败；（使）腐朽，腐烂
  // 模板匹配decay机制
  // 模板在进行匹配的时候，会进行一个退化，比如const int 如果找不到对应的类型，会退化为int。
  // 其实就是把各种引用等修饰去掉，把cosnt int&退化为int。

  // 模板类型退化 decay
  template <typename T>
  T get_max_decay(T a, T b)
  {
    std::cout << "FUNCTION NAME : " << __FUNCTION__ << ", LINE : " << __LINE__ << std::endl;
    return a > b ? a : b;
  }

  void Demo()
  {
    // 模板类型退化 decay
    // 看着比较抽象，其实就是把各种引用啊什么的修饰去掉，把cosnt int&退化为int
    int const c = 42;
    int i = 1;
    get_max_decay(i, c); // OK: T 被推断为 int，c 中的 const 被 decay 掉
    get_max_decay(c, c); // OK: T 被推断为 int

    int &ir = i;
    get_max_decay(i, ir); // OK: T 被推断为 int， ir 中的引用被 decay 掉

    // ...
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
namespace InlTest
{
class inlFileTestClass
{
public:
  void foo();
};

#include "baseTemplate.inl" // 包含 .inl 文件
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 模板匹配 问题
namespace Fit
{
  template <typename T1, typename T2, typename T3>
  struct Test
  {
  }; // 【0】

  template <typename T1, typename T2>
  struct Test<T1, T2, int>
  {
  }; // 【1】

  template <typename T>
  struct Test<T, int, int>
  {
  }; // 【2】

  void Demo()
  {
    Test<double, int, int> t; // 匹配【2】而不是【1】
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////
namespace Fit2
{
  // 在给定的代码中，存在模板特化和模板偏特化，这可能导致匹配问题和二义性问题。
  // 具体问题如下：
  // 在代码中，存在三个模板定义，分别是【0】、【1】和【2】。它们使用了不同的模板参数列表进行定义。
  // 在函数 Demo() 中，尝试创建一个类型为 Test<int, int, int> 的对象 t。
  // 问题是，在这种情况下，根据模板特化和偏特化规则，存在多个匹配的模板定义，导致模板的选择变得不明确：
  // Test<T1, T2, int> （【1】）特化匹配了 T1 和 T2 为 int，同时 T3 为 int 的情况。
  // Test<int, int, T> （【2】）偏特化匹配了 T 为 int，同时 T1 和 T2 为 int 的情况。
  // 由于存在多个匹配，编译器无法确定应该选择哪个模板定义，从而导致编译错误或二义性错误。
  // 为了解决这个问题，可以考虑调整模板定义的顺序，或者通过重载解析规则来消除二义性。
  template <typename T1, typename T2, typename T3>
  struct Test
  {
  }; // 【0】

  template <typename T1, typename T2>
  struct Test<T1, T2, int>
  {
  }; // 【1】

  template <typename T>
  struct Test<int, int, T>
  {
  }; // 【2】

  void Demo()
  {
    // Test<int, int, int> t; // 匹配问题和二义性问题
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
  Obj<int> obj;
  Obj2<int, 2> obj2;

  int nA = 43, nB = 84;
  // test.Set(nA, nB);

  // Obj<int, 2> testB(test);

  Array<int, 5> arr; // 实例化，并创建对象
  arr.at(1) = 6;

  // 变参模板 的使用
  auto a = sum(1, 2, 3, 4);                       // 10
  auto b = sum(1.5, 2, 3l);                       // 6.5
  auto c = sum(std::string("abc"), "def", "111"); // std::string("abcdef111")
  auto d = sum(2);                                // 2

  Demo();
  Demo2();

  InlTest::inlFileTestClass inlObj;
  inlObj.foo();

  Simple::Demo();
  Decay::Demo();

  Fit::Demo();
  Fit2::Demo();

  return 0;
}
