// https://blog.csdn.net/K346K346/article/details/82179205
// 对主版本模板类、全特化类、偏特化类的调用优先级从高到低进行排序是:
// 全特化类>偏特化类>主版本模板类.

#include <vector>
#include <iostream>
using namespace std;

// 类模板
template <typename T, class N>
class TestClass
{
public:
    static bool comp(T num1, N num2)
    {
        cout << "standard class template" << endl;
        return (num1 < num2) ? true : false;
    }
};

// 对部分模板参数进行特化
template <class N>
class TestClass<int, N>
{
public:
    static bool comp(int num1, N num2)
    {
        cout << "partitial specialization" << endl;
        return (num1 < num2) ? true : false;
    }
};

// 将模板参数特化为指针
template <typename T, class N>
class TestClass<T*, N*>
{
public:
    static bool comp(T* num1, N* num2)
    {
        cout << "new partitial specialization" << endl;
        return (*num1 < *num2) ? true : false;
    }
};

// 将模板参数特化为另一个模板类
template <typename T, class N>
class TestClass<vector<T>, vector<N>>
{
public:
    static bool comp(const vector<T>& vecLeft, const vector<N>& vecRight)
    {
        cout << "to vector partitial specialization" << endl;
        return (vecLeft.size() < vecRight.size()) ? true : false;
    }
};

template <typename NumType, size_t Dimension>
struct Vec
{
    Vec(const NumType& v)
    {
        for (size_t i = 0; i < Dimension; ++i)
        {
            val[i] = v;
        }
    }

    typedef NumType value_type;
    NumType val[Dimension];
};

template <typename NumType>
struct Vec<NumType, 2>
{
    typedef NumType value_type;

    union
    {
        struct
        {
            NumType val[2];
        };
        struct
        {
            union
            {
                NumType x, s, u;
            };
            union
            {
                NumType y, t, v;
            };
        };
    };
};

typedef Vec<double, 2> Vec2;
typedef std::vector<Vec2> Vec2_Vector;
typedef std::vector<Vec2_Vector> Vec2_2D_Vector;
typedef std::vector<Vec2_2D_Vector> Vec2_3D_Vector;

int main()
{
    // 调用非特化版本
    cout << TestClass<char, char>::comp('0', '1') << endl;

    // 调用部分模板参数特化版本
    cout << TestClass<int, char>::comp(30, '1') << endl;

    // 调用模板参数特化为指针版本
    int a = 30;
    char c = '1';
    cout << TestClass<int*, char*>::comp(&a, &c) << endl;

    // 调用模板参数特化为另一个模板类版本
    vector<int> vecLeft{ 0 };
    vector<int> vecRight{ 1, 2, 3 };
    cout << TestClass<vector<int>, vector<int>>::comp(vecLeft, vecRight) << endl;
}

/*

standard class template
1
partitial specialization
1
new partitial specialization
1
to vector partitial specialization
1

*/