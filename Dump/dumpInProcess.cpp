// breakpad分为 in-process 和 out-process
// 此注释说明Breakpad库提供了两种处理崩溃转储的模式
// in-process 是指在程序内部处理崩溃转储
// out-process 是指通过外部进程来处理崩溃转储

// In-Process
// 表明当前代码使用的是In-Process模式，即崩溃转储在程序内部完成

// 代码为 Linux 版, 需转成Windows版
// 提示当前代码最初是为Linux系统编写的，需要进行修改以适配Windows系统

// 根据不同的操作系统，包含不同的Breakpad异常处理头文件
// 如果是Windows系统
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// 包含Windows版的Breakpad异常处理头文件
#include "client/windows/handler/exception_handler.h"

// 如果是苹果系统，并且使用GNU C编译器或者IBM XL C/C++编译器
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
// 包含苹果版的Breakpad异常处理头文件
#include "client/mac/handler/exception_handler.h"

// 其他情况，默认是Linux系统
#else
// 包含Linux版的Breakpad异常处理头文件
#include "client/linux/handler/exception_handler.h"
#endif

// 下面是一个被注释掉的dumpCallback函数示例，用于Windows系统
// static bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
//                          void *context, bool succeeded)
// {
//     // 打印转储文件的路径
//     printf("Dump path: %s\n", descriptor.path());
//     // 返回转储操作的结果
//     return succeeded;
// }

// 如果是Windows系统
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

// 定义Windows版的dumpCallback函数，用于处理崩溃转储的回调
static bool dumpCallback(const wchar_t* dump_path, const wchar_t* id,
    void* context, EXCEPTION_POINTERS* exinfo,
    MDRawAssertionInfo* assertion,
    bool succeeded)
{
    // 分配64字节的内存用于存储转换后的字符串
    size_t nSz = 64;
    size_t nRes;
    char* chID = (char*)malloc(nSz);
    // 将宽字符字符串转换为多字节字符串
    size_t ret = wcstombs_s(&nRes, chID, nSz, id, nSz);

    // 定义一个字符数组用于存储输出信息
    char chOut[126];

    // 如果转储操作成功
    if (succeeded)
        // 格式化输出转储文件的信息
        sprintf_s(chOut, sizeof(nRes), "dumpFile: %s.dmp", chID);
    else
        // 格式化输出转储文件创建失败的信息
        sprintf_s(chOut, sizeof(nRes), "Create dumpFile error");

    // 这里注释掉了日志输出，实际使用时可以打开
    // RDLOG_ERROR(chOut);
    // 释放之前分配的内存
    free(chID);

    // 返回转储操作的结果
    return succeeded;
}

// 下面是一个被注释掉的ExceptionHandler实例化示例，用于Windows系统
// static google_breakpad::ExceptionHandler eh(
//     L".", nullptr, dumpCallback, nullptr,
//     google_breakpad::ExceptionHandler::HANDLER_ALL);

// 如果是苹果系统，并且使用GNU C编译器或者IBM XL C/C++编译器
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))

// 定义苹果版的dumpCallback函数，用于处理崩溃转储的回调
static bool dumpCallback(const char* dump_path, const char* id, void* context, bool succeeded)
{
    // 如果转储操作成功
    if (succeeded)
    {
        // 输出迷你转储文件的信息
        std::cout << "Mini Dump file: " << id << ".dump Path: " << dump_path << std::endl;

        // 这里存在一个逻辑错误，重复了判断条件，应该移除一个
        if (succeeded)
            // 格式化输出转储文件的信息
            sprintf_s(chOut, sizeof(chOut), "dumpFile: %s.dmp in: %s", id, dump_path);
        else
            // 格式化输出转储文件创建失败的信息
            sprintf_s(chOut, sizeof(chOut), "Create dumpFile error");

        // 输出错误日志
        RDLOG_ERROR(chOut);
    }
    // 返回转储操作的结果
    return succeeded;
}

// 实例化一个ExceptionHandler对象，用于处理苹果系统的崩溃转储
static google_breakpad::ExceptionHandler eh(".", nullptr, minidumpCallback, nullptr, true, nullptr);

// 其他情况，默认是Linux系统
#else

#endif

// 定义一个crash函数，用于模拟程序崩溃
void crash()
{
    // 定义一个指向空指针的整数指针
    volatile int* a = (int*)(NULL);
    // 尝试对空指针进行写操作，会导致程序崩溃
    *a = 1;
}

// 主函数，程序的入口点
int main(int argc, char* argv[])
{
    // 实例化一个ExceptionHandler对象，用于处理Windows系统的崩溃转储
    // 第一个参数是转储文件的生成目录
    // 第二个参数是过滤回调函数，这里使用nullptr表示不使用过滤
    // 第三个参数是转储回调函数，用于处理转储结果
    // 第四个参数是上下文指针，这里使用nullptr表示不使用上下文
    // 第五个参数是是否处理所有异常，这里设置为true表示处理所有异常
    // 第六个参数是异常处理选项，这里使用nullptr表示使用默认选项
    google_breakpad::ExceptionHandler eh(L".", nullptr, dumpCallback, nullptr, true, nullptr);

    // 调用crash函数，模拟程序崩溃
    crash();

    // 返回0表示程序正常结束
    return 0;
}

// MinnidumpDescriptor设置了dump生成目录为/tmp，然后初始化了ExceptionHandler，之后crash函数模拟了程序的崩溃，在程序崩溃之后会在tmp目录下生成一个dump文件，路径是descriptor.path()
// 此注释说明了程序的整体流程和崩溃转储文件的生成逻辑