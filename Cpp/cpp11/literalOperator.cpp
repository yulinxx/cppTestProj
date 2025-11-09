// https://blog.csdn.net/craftsman1970/article/details/103101965
// 从C++11开始,C++语言引入的一项技术:通过重载operator ""(双引号)后缀运算符实现的(称为literal operator,字面量运算符).
// 一般来讲,C++或者C语言都支持类似于25L之类的写法,这里的L就是字面量运算符

// change_speed(Speed s);    // better: the meaning of s is specified
//                           // 较好:定义了s的含义
// // ...
// change_speed(2.3);        // error: no unit 错误:没有单位
// change_speed(23m / 10s);  // meters per second 米每秒

#include <iostream>
using namespace std;

/////////////////////////////////////////////////////////
// 假设我们有如下一个Distance类:
struct Distance
{
    explicit Distance(double val) :meters(val)
    {
    }
    long double meters{ 0 };
};

// 可以定义如下的字面量运算符用以支持m和km单位:
Distance operator""km(long double val)
{
    return Distance(val * 1000);
}
Distance operator""m(long double val)
{
    return Distance(val);
}
Distance operator""km(unsigned long long val)
{
    return Distance(val * 1000);
}
Distance operator""m(unsigned long long val)
{
    return Distance(val);
}

// 经过这样的定义之后,以下的代码都是合法的:
// Distance d0{ 1000 };
// Distance d1{ 1.0km };

/////////////////////////////////////////////////////////
// 定义一个Time类,这个类支持sec和hour单位:
struct Time
{
    explicit Time(double val) :seconds(val)
    {
    }
    long double seconds{ 0 };
};

Time operator""sec(long double val)
{
    return Time(val);
}
Time operator""hour(long double val)
{
    return Time(val * 3600);
}
Time operator""sec(unsigned long long val)
{
    return Time(val);
}
Time operator""hour(unsigned long long val)
{
    return Time(val * 3600);
}

/////////////////////////////////////////////////////////
// 定义支持除法运算符的Speed类
struct Speed
{
    explicit Speed(double val) :speed(val)
    {
    }
    long double speed;
};
Speed operator /(Distance d, Time t)
{
    return Speed(d.meters / t.seconds);
}

int main()
{
    Distance d0{ 1000 };
    Distance d1{ 1000.0m };
    Time t1{ 2.0hour };
    Speed s1(d1 / t1);
    std::cout << "s1=" << s1.speed << "m/s" << std::endl;
    Speed s2{ 7.8km / 1sec };
    std::cout << "s2=" << s2.speed << "m/s" << std::endl;
    return 0;
}