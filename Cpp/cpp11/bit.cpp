// C++——std::Bitset_zy2317878的博客-CSDN博客_std::bitset实现
// https://blog.csdn.net/zy2317878/article/details/80082863

// 一个bitset是用来储存诸多bit,这些元素可以用来表示两种状态:0或1,true或false
// bitset头文件引用如下:
// #include <bitset>

#include <iostream> // std::cout
#include <string>   // std::string
#include <bitset>   // std::bitset

int main()
{
    // 构造一个bitset,方法如下所示:
    // bitset< n > b;	创建一个初始值均为0的n位bitset b
    // bitset< n > b(u);	根据unsigned long数u创建一个bitset b
    // bitset< n > b(s);	根据string s中含有位数的情况创建一个bitset b
    // bitset< n > b(s, pos, n);	根据string s中创建pos位置开始n个元素的bitset b

    {
        std::bitset<16> foo;
        std::bitset<16> bar(0xfa2); // 0xfa2 = 4002 = 0000 1111 1010 0010
        std::bitset<16> baz(std::string("0101111001"));

        std::cout << "foo: " << foo << std::endl;
        std::cout << "bar: " << bar << std::endl;
        std::cout << "baz: " << baz << std::endl
            << std::endl;

        // Output:
        // foo: 0000000000000000
        // bar: 0000111110100010
        // baz: 0000000101111001
    }

    // bitset::operator[]
    // 访问一个bit元素,返回bool类型的值或者对该元素的引用reference.
    {
        std::bitset<4> foo;

        foo[1] = 1;      // 0010
        foo[2] = foo[1]; // 0110

        std::cout << "foo: " << foo << std::endl
            << std::endl;

        // Output:
        // foo: 0110
    }

    // --- bitset::set()
    // 设定bitset中某一个元素或者所有元素为1
    // --- bitset::reset()
    // 将bitset中某一个元素或者所有元素重置为0,
    // --- bitset::flip()
    // 反转一个bitset,即将一个bitset中所有元素0置1,1置0
    // --- bitset::to_string()
    // 将一个bitset转换为string类型
    {
        std::bitset<4> foo;

        std::cout << foo.set() << std::endl;     // 1111
        std::cout << foo.set(2, 0) << std::endl; // 1011
        std::cout << foo.set(2) << std::endl;    // 1111
        std::cout << foo.reset(2) << std::endl;  // 1011
        std::cout << foo.flip(2) << std::endl;   // 1111
        std::cout << foo.flip() << std::endl;    // 0000

        std::string mystring =
            foo.to_string<char, std::string::traits_type, std::string::allocator_type>();

        std::cout << "mystring: " << mystring << std::endl;
        std::cout << std::endl;
        // Output:
        // 1111
        // 1011
        // 1111
        // 1011
        // 1111
        // 0000
        // mystring: 0000
    }

    // bitset::count()
    // 统计bitset中被设置的bits的个数,也就是bitset中值为1的元素个数.对于要求bitset中元素总个数,可以参考:set::size()
    {
        std::bitset<8> foo(std::string("10110011"));

        std::cout << foo << " has ";
        std::cout << foo.count() << " ones and ";
        std::cout << (foo.size() - foo.count()) << " zeros.\n"
            << std::endl;

        // Output:
        // 10110011 has 5 ones and 3 zeros.
    }

    // itset::size()
    // 返回bitset中元素总个数.即通常意义上bitset的大小.
    {
        std::bitset<8> foo;
        std::bitset<4> bar;

        std::cout << "foo.size() is " << foo.size() << std::endl;
        std::cout << "bar.size() is " << bar.size() << std::endl
            << std::endl;

        // Output:
        // foo.size() is 8
        // bar.size() is 4
    }

    // bitset::test()
    // 返回pos位置的元素是否被设置,或者是否为1.返回值为true,或false.
    {
        std::bitset<5> foo(std::string("01011"));

        std::cout << "foo contains:\n";
        std::cout << std::boolalpha;
        for (std::size_t i = 0; i < foo.size(); ++i)
            std::cout << foo.test(i) << std::endl;

        std::cout << std::endl;
        // Output:
        // foo contains:
        // true
        // true
        // false
        // true
        // false
    }

    // bitset::any()
    // 判断是否任何一个元素被设置,或者判断是否至少有一个元素为1.
    {
        std::bitset<16> foo = 0x0b; // 0x0b = 11 = 0000 1011

        if (foo.any())
            std::cout << foo << " has " << foo.count() << " bits set.\n";
        else
            std::cout << foo << " has no bits set.\n";

        std::cout << std::endl;
        //  output:
        // 0000000000001011 has 3 bits set.
    }

    // bitset::none()
    // 判断一个bitset是否没被set.如果一个bitset中有元素为1,则返回false,否则返回true
    {
        std::bitset<16> foo = 0x4f; // 0x4f = 79 = 0100 1111

        // std::cout << "Please, enter a binary number: ";
        // std::cin >> foo;

        if (foo.none())
            std::cout << foo << " has no bits set.\n";
        else
            std::cout << foo << " has " << foo.count() << " bits set.\n";

        std::cout << std::endl;
        // output:
        // 0000000001001111 has 5 bits set.
    }

    // bitset::all()
    // 判断一个bitset是否里面所有元素都为1,如果都为1,则返回true;否则返回false
    {
        std::bitset<8> foo = 0b10110101;

        // std::cout << "Please, enter an 8-bit binary number: ";
        // std::cin >> foo;

        std::cout << std::boolalpha;
        std::cout << "all: " << foo.all() << std::endl;
        std::cout << "any: " << foo.any() << std::endl;
        std::cout << "none: " << foo.none() << std::endl;
        std::cout << std::endl;
        // output:
        // all: false
        // any: true
        // none: false
    }

    return 0;
}