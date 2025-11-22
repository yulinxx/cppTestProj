// Boost功能测试文件
#include <iostream>
#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>

int main()
{
    // 打印Boost版本信息
    std::cout << "Boost版本: " << BOOST_LIB_VERSION << std::endl;
    std::cout << "Boost版本号: " << BOOST_VERSION << std::endl;

    // 测试Boost.Filesystem
    std::cout << "\n测试Boost.Filesystem:" << std::endl;
    boost::filesystem::path current_path = boost::filesystem::current_path();
    std::cout << "当前工作目录: " << current_path.string() << std::endl;
    std::cout << "是否为目录: " << (boost::filesystem::is_directory(current_path) ? "是" : "否") << std::endl;

    // 测试Boost.StringAlgorithms
    std::cout << "\n测试Boost.StringAlgorithms:" << std::endl;
    std::string test_string = "Hello Boost World!";
    std::cout << "原始字符串: " << test_string << std::endl;

    // 转小写
    boost::algorithm::to_lower(test_string);
    std::cout << "转小写后: " << test_string << std::endl;

    // 分割字符串
    std::vector<std::string> tokens;
    boost::algorithm::split(tokens, test_string, boost::is_any_of(" "));
    std::cout << "分割后的单词: " << std::endl;
    for (const auto& token : tokens)
    {
        std::cout << "- " << token << std::endl;
    }

    std::cout << "\nBoost配置测试成功!" << std::endl;
    return 0;
}