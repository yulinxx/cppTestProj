// 正则表达式描述了一种字符串匹配的模式。一般使用正则表达式主要是实现下面三个需求：
// 检查一个串是否包含某种形式的子串；
// 将匹配的子串替换；
// 从某个串中取出符合条件的子串。

// C++11 正则表达式库支持一系列的正则表达式操作，例如：
// std::regex_match：执行全字符串匹配。
// std::regex_replace：执行替换操作。
// std::smatch：保存匹配结果的容器类。
// std::regex_iterator 和 std::sregex_iterator：迭代器用于遍历匹配结果。

// C++11 提供的正则表达式库操作 std::string 对象，
// 对模式 std::regex (本质是 std::basic_regex)进行初始化，
// 通过 std::regex_match 进行匹配，从而产生 std::smatch （本质是 std::match_results 对象）。

// 我们通过一个简单的例子来简单介绍这个库的使用。考虑下面的正则表达式：

// [a-z]+.txt: 在这个正则表达式中, [a-z] 表示匹配一个小写字母,
// + 可以使前面的表达式匹配多次，因此 [a-z]+ 能够匹配一个及以上小写字母组成的字符串。
// 在正则表达式中一个 . 表示匹配任意字符，而 . 转义后则表示匹配字符 . ，
// 最后的 txt 表示严格匹配 txt 这三个字母。因此这个正则表达式的所要匹配的内容就是文件名为纯小写字母的文本文件。
// std::regex_match 用于匹配字符串和正则表达式，有很多不同的重载形式。
// 最简单的一个形式就是传入std::string 以及一个 std::regex 进行匹配，
// 当匹配成功时，会返回 true，否则返回 false。例如：

#include <iostream>
#include <string>
#include <regex>    // 引入了正则表达式相关的库

int main()
{
    std::string fnames[] = { "foo.txt", "bar.txt", "test", "a0.txt", "AAA.txt" };
    // 在 C++ 中 \ 会被作为字符串内的转义符，为使 \. 作为正则表达式传递进去生效，
    // 需要对 \ 进行二次转义，从而有 \\.

    // std::regex_match：执行全字符串匹配。

    std::cout << " ----------- " << std::endl;

    std::regex txt_regex("[a-z]+\\.txt");
    for (const auto& fname : fnames)
        std::cout << fname << ": " << std::regex_match(fname, txt_regex) << std::endl;

    std::cout << " ----------- " << std::endl;

    // 另一种常用的形式就是依次传入 std::string/std::smatch/std::regex 三个参数，其中 std::smatch 的本质其实是 std::match_results，在标准库中， std::smatch 被定义为了 std::match_results，也就是一个子串迭代器类型的 match_results。使用 std::smatch 可以方便的对匹配的结果进行获取，例如：

    std::regex base_regex("([a-z]+)\\.txt");
    std::smatch base_match;
    for (const auto& fname : fnames)
    {
        if (std::regex_match(fname, base_match, base_regex))
        {
            // sub_match 的第一个元素匹配整个字符串
            // sub_match 的第二个元素匹配了第一个括号表达式
            if (base_match.size() == 2)
            {
                std::string base = base_match[1].str();
                std::cout << "sub-match[0]: " << base_match[0].str() << std::endl;
                std::cout << fname << " sub-match[1]: " << base << std::endl;
            }
        }
    }
    std::cout << " ----------- " << std::endl;

    return 0;
}

// 以上两个代码段的输出结果为：
//  -----------
// foo.txt: 1
// bar.txt: 1
// test: 0
// a0.txt: 0
// AAA.txt: 0
//  -----------
// sub-match[0]: foo.txt
// foo.txt sub-match[1]: foo
// sub-match[0]: bar.txt
// bar.txt sub-match[1]: bar
//  -----------

// C++11 引入的正则表达式库 std::regex 提供了一系列的正则表达式操作，可以用于匹配、查找、替换和分割字符串等操作。
// 下面列出了一些常用的正则表达式操作函数：

// 1. std::regex_match：执行全字符串匹配。
// bool std::regex_match(const std::string& str, const std::regex& pattern);
// 这个函数用于检查整个字符串是否与指定的正则表达式完全匹配。

// 2. std::regex_search：执行子字符串匹配。
// bool std::regex_search(const std::string& str, const std::regex& pattern);
// 这个函数用于在字符串中查找是否存在与指定的正则表达式匹配的子字符串。

// 3. std::regex_replace：执行替换操作。
// std::string std::regex_replace(const std::string& str,
//  const std::regex& pattern, const std::string& replacement);
// 这个函数用于将与正则表达式匹配的部分替换为指定的字符串。

// 4. std::regex_iterator 和 std::sregex_iterator：迭代器用于遍历匹配结果。
// std::regex_iterator<std::string::const_iterator> std::regex_iterator;
// std::sregex_iterator std::sregex_iterator;

// 这些迭代器可以用于遍历一个字符串中与指定的正则表达式匹配的所有子字符串。
// std::regex_iterator 适用于普通字符串，std::sregex_iterator 适用于 std::string 类型。

// 5. std::smatch：保存匹配结果的容器类。

// std::smatch std::smatch;

// 这个类用于保存正则表达式的匹配结果，包括匹配的子字符串和子表达式的匹配结果。

// 以上是一些常用的正则表达式操作函数和类。C++11 正则表达式库还提供了其他一些功能和选项，如匹配标志和字符转义。