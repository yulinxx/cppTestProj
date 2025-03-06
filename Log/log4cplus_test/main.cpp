// #include <log4cplus/initializer.h>
// #include <log4cplus/logger.h>
// #include <log4cplus/loggingmacros.h>
// #include <log4cplus/configurator.h>

// int main(int argc, char *argv[])
// {
//         log4cplus::BasicConfigurator config;
//         config.configure();
//         log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("test"));
//         logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);

//         LOG4CPLUS_INFO(logger, "Hello world");
// }

#include <iostream>
#include "xxLog.h"

int main()
{
    char szData[128] = "此文件包含 main 函数。程序执行将在此处开始并结束";
    for (int i = 0; i < 100; i++)
    {
        XXLOG_DEBUG(szData);
    }

    std::cout << "Hello World!\n";
}