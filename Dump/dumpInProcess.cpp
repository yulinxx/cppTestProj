// breakpad分为 in-process 和 out-process
// In-Process

// 代码为 Linux 版, 需转成Windows版

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "client/windows/handler/exception_handler.h"

#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#include "client/mac/handler/exception_handler.h"

#else
#include "client/linux/handler/exception_handler.h"
#endif

// static bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
//                          void *context, bool succeeded)
// {
//     printf("Dump path: %s\n", descriptor.path());
//     return succeeded;
// }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

static bool dumpCallback(const wchar_t* dump_path, const wchar_t* id,
    void* context, EXCEPTION_POINTERS* exinfo,
    MDRawAssertionInfo* assertion,
    bool succeeded)
{
    size_t nSz = 64;
    size_t nRes;
    char* chID = (char*)malloc(nSz);
    size_t ret = wcstombs_s(&nRes, chID, nSz, id, nSz);

    char chOut[126];

    if (succeeded)
        sprintf_s(chOut, sizeof(nRes), "dumpFile: %s.dmp", chID);
    else
        sprintf_s(chOut, sizeof(nRes), "Create dumpFile error");

    // RDLOG_ERROR(chOut);
    free(chID);

    return succeeded;
}

// static google_breakpad::ExceptionHandler eh(
//     L".", nullptr, dumpCallback, nullptr,
//     google_breakpad::ExceptionHandler::HANDLER_ALL);

#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))

static bool dumpCallback(const char* dump_path, const char* id, void* context, bool succeeded)
{
    if (succeeded)
    {
        std::cout << "Mini Dump file: " << id << ".dump Path: " << dump_path << std::endl;

        if (succeeded)
            sprintf_s(chOut, sizeof(chOut), "dumpFile: %s.dmp in: %s", id, dump_path);
        else
            sprintf_s(chOut, sizeof(chOut), "Create dumpFile error");

        RDLOG_ERROR(chOut);
    }
    return succeeded;
}

static google_breakpad::ExceptionHandler eh(".", nullptr, minidumpCallback, nullptr, true, nullptr);

#else

#endif

void crash()
{
    volatile int* a = (int*)(NULL);
    *a = 1;
}

int main(int argc, char* argv[])
{
    // google_breakpad::ExceptionHandler eh(L"./", NULL, dumpCallback, NULL, true, -1);
    google_breakpad::ExceptionHandler eh(L".", nullptr, dumpCallback, nullptr, true, nullptr);

    crash();

    return 0;
}

// MinnidumpDescriptor设置了dump生成目录为/tmp，然后初始化了ExceptionHandler，之后crash函数模拟了程序的崩溃，在程序崩溃之后会在tmp目录下生成一个dump文件，路径是descriptor.path()