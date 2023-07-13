#include <string>
// https://blog.csdn.net/c1sdn19/article/details/123669618

// 只有编译期能够确定的量，才能用来实例化模板。o

// 模板特化（template specialization）不同于模板的实例化，模板参数在某种特定类型下的具体实现称为模板特化。
// 模板特化有时也称之为模板的具体化，分别有函数模板特化和类模板特化。

// 模板是编译期语法，因此，这里的整型数据也必须是编译期能确定的，比如说常数、常量表达式等，而不可以是动态的数据。
// 在C++20以前，只允许整数参数(本质上是整数的类型)，但从C++20起，可以支持浮点数做参数

// C++的模板并非直接可用的代码，需要经历「实例化」，而实例化的过程其实就是指定参数的过程。

template <typename NumType>
class Obj
{
};
// template <typename NumType>
// class Obj1<NumType, 9>
// {
// public:
//     NumType array[]
// };

// 偏特化模板
template <typename NumType, int count = 9>
class Obj2
{
public:
    NumType array[count];
};

template <typename T, typename N, int d = 9>
class Obj3
{
public:
    T arrayA[3];
    N arrayB[3];
};

// template <typename T,  size_t size>
// class Array
// {
// public:
//     Array() {}
//     T &at(int n) {return data_[n];}
//     size_t size() const {}

// private:
//     T data_[size];
// };

// 函数类型模板参数
template <void func()>
void f()
{
    func();
}

// 普通函数
void f1() {}

class Test
{
public:
    void f2() {}        // 成员函数
    static void f3() {} // 静态成员函数
};

// 变参模板 https://blog.csdn.net/fl2011sx/article/details/128077440
template <typename... Args>
auto sum(Args... args)
{
    return (... + args);
}


// 整数参数的模板
template <int N>
struct Test {};

void Demo() {
  Test<5> t1; // 常数OK

  constexpr int a = 5;
  Test<a> t2; // 常量表达式OK

  const int b = 6;
  Test<b> t3; // ERR，b是只读变量，不是常量

  Test<a * 3> t4; // 常数运算OK

  std::vector<int> ve {1, 2, 3};
  Test<ve.size()> t5; // ERR，size是运行时数据
  Test<ve[1]> t6; // ERR，ve的成员是运行时数据

  int arr1[] {1, 2, 3};
  Test<arr1[0]> t6; // ERR，arr1的成员是运行时数据

  constexpr int arr2[] {2, 4, 6};
  Test<arr2[1]> t7; // 常量表达式修饰的普通数组成员OK
}



int a = 1; // 全局变量

class Test {
 public:
  int m1; // 成员变量
  static int m2; // 静态成员变量
};

int Test::m2 = 4;

void Demo1() {
  int b = 2; // 局部变量
  static int c = 3; // 静态局部变量
  
  f<&a>(0); // OK，a是全局变量，程序运行期间不会被释放
  f<&b>(0); // ERR，b是局部变量，在局部代码块运行完毕后会被释放，所以说b的地址也有可能不仅仅表示b，回收后可能会表示其他数据，所以不可以
  f<&c>(0); // OK，c是静态局部变量，不会随着代码块的结束而释放
  
  f<&Test::m1>(0); // ERR，Test::m1其实并不是变量，要指定了对象才能确定，因此是非确定值，所以不可以
  f<&Test::m2>(0); // OK，Test::m1本质就是一个全局变量，在程序运行期间不会被释放，所以OK
}


// 「模板类型的模板参数」，其实就是嵌套模板的意思，把「某一种类型的模板」作为一个参数传给另一个模板
// 模板类型的模板参数
template <template <typename, typename> typename Tem>
void f() {
  Tem<int, std::string> te;
  te.show();
}

// 符合条件的模板类
template <typename T1, typename T2>
struct Test1 {
  void show() {std::cout << 1 << std::endl;}
};

template <typename T1, typename T2>
struct Test2 {
  void show() {std::cout << 2 << std::endl;}
};

// 不符合条件的模板类
template <int N>
struct Test3 {
  void show() {std::cout << 3 << std::endl;}
};

void Demo2() {
  f<Test1>(); // 注意这里，要传模板，而不是实例化后的模板
  f<Test<int, int>>(); // ERR，模板参数类型不匹配
  
  f<Test2>(); // OK
  f<Test3>(); // ERR，类型不匹配
}


int main()
{
    Obj<int> obj;
    Obj2<int, 2> obj2;
    Obj3<int, float, 3> obj3;

    // Obj<std::string, 2> objC();
    // Obj<int, 4> objD;

    int nA = 43, nB = 84;
    // test.Set(nA, nB);

    // Obj<int, 2> testB(test);


    // Array<int, 5> arr; // 实例化，并创建对象
    // arr.at(1) = 6;

    // 函数类型模板参数 使用
    void (*pf1)() = &f1;           // 局部变量
    constexpr void (*pf2)() = &f1; // 常量表达式

    f<f1>();        // OK
    f<&f1>();       // OK
    f<Test::f3>();  // OK
    f<&Test::f3>(); // OK
    f<pf2>();       // OK

    // 变参模板 的使用
    auto a = sum(1, 2, 3, 4);                       // 10
    auto b = sum(1.5, 2, 3l);                       // 6.5
    auto c = sum(std::string("abc"), "def", "111"); // std::string("abcdef111")
    auto d = sum(2);                                // 2
    return 0;
}
