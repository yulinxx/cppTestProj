#include <iostream>

#include <cmath>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

//#include "numeric_limits.hpp"
#include <limits>
#include <iostream>

int main()
{
    // 对多个值取最值
    int mi = std::min(32, -96);
    int ma = std::max(54, 32);
    int mi2 = std::min({ 32, 45, 1, 0, -98, 35 });

    // time
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << "s";

    auto startB = std::chrono::system_clock::now();
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    auto endB = std::chrono::system_clock::now();

    auto elapsedB = endB - startB;
    std::cout << "Elapsed time: " << elapsedB.count() << "s";

    //
    // auto让编译器通过初始值来推算变量的类型.当然,其定义的变量必须要有初始值
    auto a = 1;
    // decltype(变量)可以获取变量的类型
    decltype(a) b = 2;

    // 注意下,decltype((a))的结果是引用,此时创建新的变量就将会报错,或者说:
    //  int &b = a;
    //  decltype(b) c;//也报错,因为b是a的引用,decltype(b)就会报错,效果同decltype((a))

    // Lambda
    //  Lambda,基本形式如下:
    // [](argument1,argument2,.....){//code}
    // 在 () 中传入参数,在 {} 中编写代码,[] 是一个捕获列表,可以指定外部作用域中,可以使用的局部变量:
    // [] — 捕获列表为空,表示在 lambda 表达式中不能使用任何外部作用域的局部变量,只能使用传进去的参数.
    // [=] — 表示按照值传递的方法捕获父作用域的所有变量.
    // [&] — 表示按照应用传递的方法捕获父作用域的所有变量.
    // [this] — 在成员函数中,按照值传递的方法捕获 this 指针.
    // [a, &b] — 不建议直接使用 [=] 或 [&] 捕获所有变量,可以按需显式捕获,就是在 [] 指定变量名,[a] 表示值传递,[&b] 表示引用传递.
    std::vector<int> v;
    v = { 1, 53, 17, 25 };
    // sort
    std::sort(v.begin(), v.end(), [](int a, int b) { return (a % 10) > (b % 10); });

    // 用 emplace_back 代替 push_back , emplace_back 就直接在 vector 的末尾构造值(构造函数),少调用了一次拷贝构造函数.
    {
        std::vector<int> v;
        v.push_back(0);
        v.emplace_back(1);
    }

    // 取整  C/C++取整函数ceil(向上取整)和floor(向下取整) 四舍五入round
    std::cout << "\n-----------------------" << std::endl;
    std::cout << "std::ceil(1.2):" << std::ceil(1.2) << std::endl;   // 1.2向上取整 std::ceil(1.2):2
    std::cout << "std::ceil(0.0):" << std::ceil(0.0) << std::endl;   // 0.0向上取整 std::ceil(0.0):0
    std::cout << "std::ceil(-1.2):" << std::ceil(-1.2) << std::endl; // -1.2向上取整 std::ceil(-1.2):-1

    std::cout << "std::floor(1.2)" << std::floor(1.2) << std::endl;     // std::ceil(0.0)   std::floor(1.2)1
    std::cout << "std::floor(0.0):" << std::floor(0.0) << std::endl;   // 0.0向下取整   std::floor(0.0):0
    std::cout << "std::floor(-1.2):" << std::floor(-1.2) << std::endl; // std::ceil(0.0)    std::floor(-1.2):-2

    std::cout << "std::round(1.2):" << std::round(1.4) << std::endl;   // std::ceil(0.0)    std::round(1.2):1
    std::cout << "std::round(1.5):" << std::round(1.5) << std::endl;   // std::ceil(0.0)    std::round(1.5):2
    std::cout << "std::round(0.0):" << std::round(0.0) << std::endl;   // std::ceil(0.0)    std::round(0.0):0
    std::cout << "std::round(-1.4):" << std::round(-1.4) << std::endl; // std::ceil(0.0)    std::round(-1.4):-1
    std::cout << "std::round(-1.5):" << std::round(-1.5) << std::endl; // std::ceil(0.0)    std::round(-1.5):-2

    // 在C/C++11中,std::numeric_limits为模板类,在库编译平台提供基础算术类型的极值等属性信息.
    // 用于取代<climits>和<limits.h>,浮点常数定义于<cfloat>和<float.h>.
    std::cout << std::boolalpha;
    std::cout << "Minimum value for int: " << std::numeric_limits<int>::min() << std::endl;
    std::cout << "Maximum value for int: " << std::numeric_limits<int>::max() << std::endl;
    std::cout << "int is signed: " << std::numeric_limits<int>::is_signed << std::endl;
    std::cout << "Non-sign bits in int: " << std::numeric_limits<int>::digits << std::endl;
    std::cout << "int has infinity: " << std::numeric_limits<int>::has_infinity << std::endl;

    std::cout << "Minimum value for float: " << std::numeric_limits<float>::min() << std::endl; // min returns the smallest positive value the type can encode, not the lowest
    std::cout << "Lowest value for float: " << std::numeric_limits<float>::lowest() << std::endl; // the lowest value
    std::cout << "Maximum value for float: " << std::numeric_limits<float>::max() << std::endl;
    std::cout << "float is signed: " << std::numeric_limits<float>::is_signed << std::endl;
    std::cout << "Non-sign bits in float: " << std::numeric_limits<float>::digits << std::endl;
    std::cout << "float has infinity: " << std::numeric_limits<float>::has_infinity << std::endl;

    std::cout << "Minimum value for unsigned short: " << std::numeric_limits<unsigned short>::min() << std::endl;
    std::cout << "Maximum value for unsigned short: " << std::numeric_limits<unsigned short>::max() << std::endl;

    std::cout << "is_specialized(float): " << std::numeric_limits<float>::is_specialized << std::endl;
    std::cout << "is_integer(float): " << std::numeric_limits<float>::is_integer << std::endl;
    std::cout << "is_exact(float): " << std::numeric_limits<float>::is_exact << std::endl;
    std::cout << "is_bounded(float): " << std::numeric_limits<float>::is_bounded << std::endl;
    std::cout << "is_modulo(float): " << std::numeric_limits<float>::is_modulo << std::endl;
    std::cout << "is_iec559(float): " << std::numeric_limits<float>::is_iec559 << std::endl;
    std::cout << "digits10(float): " << std::numeric_limits<float>::digits10 << std::endl;
    std::cout << "radix(float): " << std::numeric_limits<float>::radix << std::endl;
    std::cout << "min_exponent(float): " << std::numeric_limits<float>::min_exponent << std::endl;
    std::cout << "max_exponent(float): " << std::numeric_limits<float>::max_exponent << std::endl;
    std::cout << "min_exponent10(float): " << std::numeric_limits<float>::min_exponent10 << std::endl;
    std::cout << "max_exponent10(float): " << std::numeric_limits<float>::max_exponent10 << std::endl;
    std::cout << "epsilon(float): " << std::numeric_limits<float>::epsilon() << std::endl;
    std::cout << "round_style(float): " << std::numeric_limits<float>::round_style << std::endl;

    std::cout << "The smallest nonzero denormalized value for float: "
        << std::numeric_limits<float>::denorm_min() << std::endl;
    std::cout << "The difference between 1 and the smallest value greater than 1 for float: "
        << std::numeric_limits<float>::epsilon() << std::endl;
    std::cout << "Whether float objects allow denormalized values: "
        << std::numeric_limits<float>::has_denorm << std::endl;
    std::cout << "Whether float objects can detect denormalized loss: "
        << std::numeric_limits<float>::has_denorm_loss << std::endl;
    std::cout << "Whether float objects have quiet_NaN: "
        << std::numeric_limits<float>::has_quiet_NaN << std::endl;
    std::cout << "Whether float objects have a signaling_NaN: "
        << std::numeric_limits<float>::has_signaling_NaN << std::endl;
    std::cout << "The base for type float is:  "
        << std::numeric_limits<float>::radix << std::endl;
    std::cout << "The maximum rounding error for type float is:  "
        << std::numeric_limits<float>::round_error() << std::endl;
    std::cout << "The rounding style for a double type is: "
        << std::numeric_limits<double>::round_style << std::endl;
    std::cout << "The signaling NaN for type float is:  "
        << std::numeric_limits<float>::signaling_NaN() << std::endl;
    std::cout << "Whether float types can detect tinyness before rounding: "
        << std::numeric_limits<float>::tinyness_before << std::endl;
    std::cout << "Whether float types have implemented trapping: "
        << std::numeric_limits<float>::traps << std::endl;

    return 0;
}