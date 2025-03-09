// std::map 在构造时,默认使用一个比较函数对象来进行排序.这个比较函数对象被称为"键比较函数"(Key Comparison Function)或"比较器"(Comparator).
// 该比较器用于确定两个键的顺序,以决定它们在 std::map 中的位置.

// std::map 的构造函数接受一个可选的比较器参数,它指定了如何对键进行排序.
// 如果未提供比较器参数,则使用默认的比较器 std::less<Key>,其中 Key 是 std::map 的键类型.
// std::less<Key> 是一个函数对象,它按照键的自然顺序进行比较.

// 如果你想要使用自定义的排序方式,可以提供一个比较器对象作为 std::map 的构造函数参数.
// 比较器对象是一个可调用的函数对象,它接受两个键作为参数,并返回一个布尔值,指示键的顺序.
// 比较器对象应该定义一个严格的弱顺序(Strict Weak Ordering).

#include <iostream>
#include <map>
#include <string>

struct LengthComparator
{
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        return lhs.length() < rhs.length();
    }
};

struct LengthComparatorX
{
    bool operator()(const int lhs, const int rhs) const
    {
        return lhs > rhs;
    }
};

int main()
{
    std::map<std::string, int, LengthComparator> myMap;
    std::map<int, std::string, LengthComparatorX> myMapX;

    myMap["apple"] = 1;
    myMap["banana"] = 2;
    myMap["orange"] = 3;

    myMapX[1] = "aaaa";
    myMapX[2] = "bbbbb";
    myMapX[3] = "ccccc";
    myMapX[4] = "ddddd";

    for (const auto& pair : myMap)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    for (const auto& pair : myMapX)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    return 0;
}