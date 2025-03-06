#ifndef _XX_LOG_H_
#define _XX_LOG_H_

#include "log4cplus/loglevel.h"
#include "log4cplus/ndc.h"
#include "log4cplus/logger.h"
#include "log4cplus/configurator.h"
#include "iomanip"
#include "log4cplus/fileappender.h"
#include "log4cplus/layout.h"
#include <log4cplus/loggingmacros.h>
#include <sstream>
#include <iostream>
#include <log4cplus/log4cplus.h>
#include <log4cplus/consoleappender.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

#include <memory>

#define PATH_SIZE 128

#define LOG4CPLUS_CONFIG_FILE "log.properties"

// /// 记录追踪日志
// #define LOG_TRACE(msg)                            \
//     {                                             \
//         xxLog::instance();                    \
//         LOG4CPLUS_TRACE(xxLog::_logger, msg); \
//     }
// /// 记录调试日志
// #define LOG_DEBUG(msg)                            \
//     {                                             \
//         xxLog::instance();                    \
//         LOG4CPLUS_DEBUG(xxLog::_logger, msg); \
//     }
// /// 记录信息日志
// #define LOG_INFO(msg)                            \
//     {                                            \
//         xxLog::instance();                   \
//         LOG4CPLUS_INFO(xxLog::_logger, msg); \
//     }
// /// 记录告警日志
// #define LOG_WARN(msg)                            \
//     {                                               \
//         xxLog::instance();                      \
//         LOG4CPLUS_WARN(xxLog::_logger, msg); \
//     }
// /// 记录错误日志
// #define LOG_ERROR(msg)                            \
//     {                                             \
//         xxLog::instance();                    \
//         LOG4CPLUS_ERROR(xxLog::_logger, msg); \
//     }
// /// 记录致命日志
// #define LOG_FATAL(msg)                            \
//     {                                             \
//         xxLog::instance();                    \
//         LOG4CPLUS_FATAL(xxLog::_logger, msg); \
//     }


/////////////////////////////////////////
// ------ 日志管理 ------ 
// 打开日志,记录所有信息
#define XXLOG_ON()   \
{ \
	log4cplus::initialize();	\
	xxLog::m_logger.setLogLevel(ALL_LOG_LEVEL); \
}

// 关闭日志
#define XXLOG_OFF()   \
{ \
	xxLog::m_logger.setLogLevel(OFF_LOG_LEVEL); \
}

// 记录所有日志 
#define XXLOG_REC_ALL()   \
{ \
	xxLog::m_logger.setLogLevel(ALL_LOG_LEVEL); \
}

// // 仅记录调试与错误日志 
#define XXLOG_REC_DEBUG()   \
{ \
	xxLog:m_logger.:setLogLevel(DEBUG_LOG_LEVEL); \
}

// 仅记录错误日志 
#define XXLOG_REC_ERROR()   \
{ \
	xxLog::m_logger.setLogLevel(ERROR_LOG_LEVEL); \
}

// 彻底关闭日志功能(在程序退出时调用)
#define XXLOG_CLOSE()\
{ \
	log4cplus::deinitialize();	\
}

// ------ 日志写入 ------ 
// 追踪日志
#define XXLOG_TRACE(msg)   \
{ \
	xxLog::instance(); \
	LOG4CPLUS_TRACE(xxLog::m_logger, msg); \
}

// 调试日志
#define XXLOG_DEBUG(msg)   \
{ \
	xxLog::instance(); \
	LOG4CPLUS_DEBUG(xxLog::m_logger, msg); \
}

// 信息日志
#define XXLOG_INFO(msg)    \
{ \
	xxLog::instance();\
	LOG4CPLUS_INFO(xxLog::m_logger, msg);\
}

// 警告日志
#define XXLOG_WARN(msg) \
{ \
	xxLog::instance(); \
	LOG4CPLUS_WARN(xxLog::m_logger, msg); \
}

// 错误日志
#define XXLOG_ERROR(msg)   \
{ \
	xxLog::instance(); \
	LOG4CPLUS_ERROR(xxLog::m_logger, msg); \
}

// 致命日志
#define XXLOG_FATAL(msg)  \
{ \
	xxLog::instance(); \
	LOG4CPLUS_FATAL(xxLog::m_logger, msg); \
}


///////////////////////////////////////

using namespace std;
class xxLog
{
public:
    static xxLog *instance();

    static Logger m_logger;

public:
    static void closeLog();

private:
    xxLog();
    virtual ~xxLog();

    static xxLog *m_lpxxLog;
};

#endif //_XX_LOG_H_