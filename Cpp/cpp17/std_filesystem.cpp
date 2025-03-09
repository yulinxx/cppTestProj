// std::filesystem 是 C++17 中引入的标准库,用于处理文件系统操作和路径管理.
// 它提供了一组类和函数,用于在 C++ 程序中执行常见的文件和目录操作,如文件/目录的创建、重命名、删除、遍历、查询属性等.

// std::filesystem 的设计目标是提供一种跨平台的、统一的、面向对象的文件系统接口,
// 使开发者能够更方便地进行文件和目录的处理,而无需依赖操作系统特定的 API.

// 以下是 std::filesystem 的一些主要功能和用法:

// 1. 路径操作:std::filesystem 提供了 std::filesystem::path 类型来表示文件系统路径.它支持路径的拼接、分解、规范化等操作.
// 2. 文件和目录操作:std::filesystem 提供了一组函数来执行文件和目录的创建、删除、重命名、复制等操作.

// 3. 文件遍历:std::filesystem 提供了一些函数来遍历目录中的文件和子目录,例如 std::filesystem::directory_iterator.
// 4. 文件属性查询:std::filesystem 提供了一组函数来查询文件和目录的属性,如文件大小、修改时间、权限等.

// 5. 文件操作异常处理:std::filesystem 抛出异常来处理文件和目录操作的错误,例如文件不存在、权限不足等.

#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
    // --- 路径操作:
    fs::path filePath = fs::current_path() / "test.txt";

    // 创建路径对象
    fs::path myPath = fs::current_path();

    // 连接路径
    fs::path newPath = myPath / "subdir" / "myfile.txt";

    // 获取路径字符串表示
    std::string pathStr = newPath.string();

    // 检查路径是否存在
    bool exists = fs::exists(newPath);

    // 获取路径的父目录
    fs::path parentPath = newPath.parent_path();

    // --- 文件和目录操作:
    // 创建目录
    fs::create_directory("mydir");

    // 创建多级目录
    fs::create_directories("path/to/mydir");

    // 创建文件
    std::ofstream file("myfile.txt");
    file << "Hello, world!";
    file.close();

    // 重命名文件
    fs::rename("oldname.txt", "newname.txt");

    // 删除文件
    fs::remove("myfile.txt");

    // 删除目录
    fs::remove_all("mydir");

    // --- 文件遍历:
    // 遍历目录中的文件和子目录
    fs::directory_iterator dirIt("mydir");
    for (const auto& entry : dirIt)
    {
        if (fs::is_directory(entry))
        {
            std::cout << "子目录: " << entry.path() << std::endl;
        }
        else if (fs::is_regular_file(entry))
        {
            std::cout << "文件: " << entry.path() << std::endl;
        }
    }

    // --- 文件属性查询:
    // 查询文件大小
    std::uintmax_t fileSize = fs::file_size("myfile.txt");

    // 查询文件修改时间
    std::filesystem::file_time_type modTime = fs::last_write_time("myfile.txt");

    // 查询文件权限
    fs::perms permissions = fs::status("myfile.txt").permissions();

    return 0;
}

// 在这个示例中,我们首先使用 std::filesystem::path 类型创建了一个文件路径 filePath.
// 然后,我们创建了一个文件并写入一些内容.通过 std::filesystem 提供的函数,我们查询了文件的大小和修改时间,并最后删除了文件.

// std::filesystem 提供了更多的功能,如递归遍历目录、创建目录、文件复制等.
// 它为 C++ 开发者提供了强大而方便的文件系统操作接口,使得文件和目录的处理更加容易和可靠.