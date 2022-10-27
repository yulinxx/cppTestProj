#include "xxLog.h"
#include <iostream>

using namespace std;
xxLog *xxLog::m_lpxxLog = nullptr;
Logger xxLog::m_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("ALL_MSGS"));

xxLog::xxLog()
{
    log4cplus::initialize();
    log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(LOG4CPLUS_CONFIG_FILE));
}

xxLog::~xxLog()
{
    log4cplus::Logger::shutdown();
}

xxLog *xxLog::instance()
{
    if (!m_lpxxLog)
    {
        m_lpxxLog = new xxLog();
    }
    return m_lpxxLog;
}

void xxLog::closeLog()
{
    log4cplus::Logger logger = log4cplus::Logger::getRoot(); 
    logger.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
    return;


    // Logger allLogger = Logger::getInstance(LOG4CPLUS_TEXT("ALL_MSGS"));
    // allLogger.shutdown();

    Logger debugLogger = Logger::getInstance(LOG4CPLUS_TEXT("ALL"));
    debugLogger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    // debugLogger.shutdown();

    Logger traceLogger = Logger::getInstance(LOG4CPLUS_TEXT("TRACE_MSGS"));
    traceLogger.setLogLevel(log4cplus::OFF_LOG_LEVEL);
}