// SFINAE(subsitate failure is not an error) (模板匹配失败不是错误)

// C++模板提供了一个SFINAE(subsitate failure is not an error)的机制(模板匹配失败不是错误),
// 这是模板里面一个非常有意思的特性,利用这个机制可以检查一个结构体是否包含某个成员等操作.
// c++语言本身没有提供反射机制(也有利用pb实现反射),利用SFINAE机制,可以实现类似于反射的功能.

#include <iostream>
#include <type_traits>
using namespace std;

///////////////////////////////////////////////////////////////////////
namespace SpaceA
{
    // https://www.jianshu.com/p/20577b8d273e
    // SFINAE 这个看着很抽象,实际非常简单,是一种常见的模板技巧.
    // 比如,利用模板的SFINAE判断一个结构体中是否包含某个成员.

    // 这段代码使用模板和SFINAE机制,通过重载解析和类型推导来判断给定的类型是否具有特定的成员变量 id.
    // 具体实现是通过定义两个重载的静态成员函数 check,其中一个版本接受 decltype(&U::id) 类型的参数,用于类型匹配,
    // 另一个版本接受任意参数类型,用于触发SFINAE.
    // 通过判断 check<T>(NULL) 的返回类型是否为 void,可以确定类型 T 是否具有成员变量 id.

    // 定义了一个模板类 check_has_member_id,接受一个类型参数 T
    template <typename T>
    struct check_has_member_id
    {
        // 仅当T是一个类类型时,U::* 才是存在的,从而这个泛型函数的实例化才是可行的, 否则,就将触发SFINAE
        // 定义了一个静态成员函数 check,接受一个参数类型为 decltype(&U::id),即指向 U 类型的 id 成员的指针.
        // 这个函数没有具体的实现,只是用于类型检查.
        template <typename U>
        static void check(decltype(&U::id))
        {}

        // 仅当触发SFINAE时,编译器才会“被迫”选择这个版本
        // 定义了另一个静态成员函数 check,接受任意参数类型,并返回整数值 0.
        // 这个函数是一个通用版本,用于处理无法匹配 decltype(&U::id) 的情况.
        template <typename U>
        static int check(...)
        {
            return 0;
        }

        // 定义了一个枚举类型 value,其值根据 std::is_void 对 check<T>(NULL) 的结果进行判断.
        // 如果 check<T>(NULL) 的返回类型是 void,则 value 的值为 true,否则为 false.
        enum
        {
            value = std::is_void<decltype(check<T>(NULL))>::value
        };
    };

    // 定义了一个结构体 TEST_STRUCT,具有一个整型成员 rid.
    struct TEST_STRUCT
    {
        int rid;
    };

    // 定义了另一个结构体 TEST_STRUCT2,具有一个整型成员 id.
    struct TEST_STRUCT2
    {
        int id;
    };

    int Demo()
    {
        // 创建一个 check_has_member_id 类型的对象 t1,并指定类型参数为 TEST_STRUCT.
        check_has_member_id<TEST_STRUCT> t1;
        cout << t1.value << endl; // 0

        // 创建一个 check_has_member_id 类型的对象 t2,并指定类型参数为 TEST_STRUCT2.
        // 输出 t2.value 的值,即类型 TEST_STRUCT2 是否具有成员变量 id 的判断结果.
        // 在这种情况下,输出为 1,表示 TEST_STRUCT2 具有 id 成员.
        check_has_member_id<TEST_STRUCT2> t2;
        cout << t2.value << endl; // 1

        // 输出 t3.value 的值,即类型 int 是否具有成员变量 id 的判断结果.
        // 在这种情况下,输出为 0,表示 int 不具有 id 成员.
        check_has_member_id<int> t3;
        cout << t3.value << endl; // 0

        return 0;
    }

    // 核心的代码是在实例化check_has_member_id对象的时候,通过模板参数T的类型,决定了结构体中对象value的值.
    // 而value的值是通过check<T>函数的返回值是否是void决定的.
    // 如果T中含有id成员的话,那么就会匹配第一个实例,返回void;不包含id,会匹配默认的实例,返回int.
}

///////////////////////////////////////////////////////////////////////
namespace SpaceB
{
    // 用于检查类型是否为类类型的模板类 check.
    // 它使用了 SFINAE 机制,通过在编译时进行重载解析来判断类型是否是类类型.
#include <iostream>
#include <type_traits>

    // 2. 判断变量是否是一个struct 或者 类
    // https://www.jianshu.com/p/d09373b83f86
    template <typename T>
    struct check
    {
        // 定义了一个静态成员函数 check_class,接受一个类型为 int U::* 的参数,
        // 该参数是一个指向 U 类型的成员的指针.
        // 这个函数没有具体的实现,只是用于类型检查.

        // int U::* 是一个指向成员的指针类型,其中 int 是成员的类型,表示指向一个 int 类型的成员.
        // U::* 表示它是指向类型为 U 的成员的指针.
        // U 是一个模板参数,在使用时将会根据实际传入的类型进行替换.
        // ::* 是指向成员的指针运算符,它用于指示这是一个指向成员的指针

        template <typename U>
        static void check_class(int U::*)
        {}

        // 定义了另一个静态成员函数 check_class,接受任意参数类型,并返回整数值 0.
        // 这个函数是一个通用版本,用于处理无法匹配 int U::* 类型的情况.
        template <typename U>
        static int check_class(...)
        {
            return 0;
        }

        // 定义了一个枚举类型 value,其值根据 std::is_void 对 check_class<T>(0) 的结果进行判断.
        // 如果 check_class<T>(0) 的返回类型是 void,则 value 的值为 true,否则为 false.
        enum
        {
            value = std::is_void<decltype(check_class<T>(0))>::value
        };
    };

    class myclass
    {
    };

    int Demo()
    {
        check<myclass> t;
        // 输出 t.value 的值,即类型 myclass 是否是类类型的判断结果.
        // 在这种情况下,输出为 1,表示 myclass 是类类型.
        std::cout << t.value << std::endl; // 1

        check<int> t2;
        // 输出 t2.value 的值,即类型 int 是否是类类型的判断结果.
        // 在这种情况下,输出为 0,表示 int 不是类类型.
        std::cout << t2.value << std::endl; // 0
        return 0;
    }

    // 整体来说,这段代码使用模板和 SFINAE 机制,通过重载解析和类型推导来判断给定的类型是否是类类型.
    // 具体实现是通过定义两个重载的静态成员函数 check_class,其中一个版本接受 int U::* 类型的参数,
    // 用于类型匹配,另一个版本接受任意参数类型,用于触发 SFINAE.
    // 通过判断 check_class<T>(0) 的返回类型是否为 void,可以确定类型 T 是否是类类型.
}

///////////////////////////////////////////////////////////////////////
namespace SpaceC
{
    // enable_if 语法
    // 如果T是一个int类型,那么返回值是bool类型.如果不是int的话,就匹配不到找个实例.
    // 使用enable_if的好处是控制函数只接受某些类型的(value==true)的参数,否则编译报错.

    // 判断class T 是否有某个成员函数
    // enable_if example: two ways of using enable_if
#include <type_traits>

    // 1. the return type (bool) is only valid if T is an integral type:
    template <class T>
    typename std::enable_if<std::is_integral<T>::value, bool>::type
        is_odd(T i)
    {
        return bool(i % 2);
    }

    int Demo()
    {
        short int i = 1;                                     // code does not compile if type of i is not integral
        std::cout << "i is odd: " << is_odd(i) << std::endl; // i is odd: 1

        // 编译错误
        // double j = 10.0;
        // std::cout << "i is odd: " << is_odd(j) << std::endl;

        return 0;
    }

    // 利用模板的这种机制,可以设计“通用”函数接口.
    // 比如,如果work_func<T>(T data) 函数传入的类型T中包含了T::is_print成员就打印“xxxx”,如果不包含就打印“yyy”.
}

///////////////////////////////////////////////////////////////////////

namespace SpaceD
{
#if 0

    // 这个事情除了可以用模板来做,利用反射也可以实现.c++本身没有提供反射,利用pb,也可以实现.下面介绍一下基于pb实现的c++反射的机制.

    int get_feature(const HeartBeatMessage& hb_msg, const std::string& name)
    {
        const google::protobuf::Descriptor* des = hb_msg.GetDescriptor();
        const google::protobuf::FieldDescriptor* fdes = des->FindFieldByName(name);
        assert(fdes != nullptr);
        const google::protobuf::Reflection* ref = hb_msg.GetReflection();
        cout << ref->GetString(hb_msg, fdes) << endl;
        return 0;
    }

    int Demo()
    {
        HeartBeatMessage msg;
        fstream input("./heartbeat.db", ios::in | ios::binary);
        if (!msg.ParseFromIstream(&input))
        {
            cerr << "read data from file error." << endl;
            return -1;
        }

        // msg 是pb的msg对象,从msg对象里面找到对应“hostname”字段
        get_feature(msg, "hostName");
    }

#endif
}
///////////////////////////////////////////////////////////////////////
int main()
{
    std::cout << "--- SpaceA --- " << std::endl;
    SpaceA::Demo();

    std::cout << "--- SpaceB --- " << std::endl;
    SpaceB::Demo();

    std::cout << "--- SpaceC --- " << std::endl;
    SpaceC::Demo();

    return 0;
}
// g++ --std=c++11  xxx.c