#pragma once

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

/// 记录追踪日志
#define LOG_TRACE(msg)                            \
    {                                             \
        CLogUtils::instance();                    \
        LOG4CPLUS_TRACE(CLogUtils::_logger, msg); \
    }
/// 记录调试日志
#define LOG_DEBUG(msg)                            \
    {                                             \
        CLogUtils::instance();                    \
        LOG4CPLUS_DEBUG(CLogUtils::_logger, msg); \
    }
/// 记录信息日志
#define LOG_INFO(msg)                            \
    {                                            \
        CLogUtils::instance();                   \
        LOG4CPLUS_INFO(CLogUtils::_logger, msg); \
    }
/// 记录告警日志
#define LOG_WARNING(msg)                            \
    {                                               \
        CLogUtils::instance();                      \
        LOG4CPLUS_WARNING(CLogUtils::_logger, msg); \
    }
/// 记录错误日志
#define LOG_ERROR(msg)                            \
    {                                             \
        CLogUtils::instance();                    \
        LOG4CPLUS_ERROR(CLogUtils::_logger, msg); \
    }
/// 记录致命日志
#define LOG_FATAL(msg)                            \
    {                                             \
        CLogUtils::instance();                    \
        LOG4CPLUS_FATAL(CLogUtils::_logger, msg); \
    }

using namespace std;
class CLogUtils
{
public:
    static CLogUtils *instance();

    static Logger _logger;

private:
    CLogUtils();
    virtual ~CLogUtils();

    static CLogUtils *m_lpCLogUtils;
};