#include "LogUtils.h"
#include <iostream>
 
using namespace std;
CLogUtils* CLogUtils::m_lpCLogUtils = nullptr;
Logger CLogUtils::_logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("ALL_MSGS"));
 
CLogUtils::CLogUtils()
{
    log4cplus::initialize();
    log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(LOG4CPLUS_CONFIG_FILE));
}
 
CLogUtils::~CLogUtils()
{
    log4cplus::Logger::shutdown();
}
 
CLogUtils* CLogUtils::instance()
{
    if (!m_lpCLogUtils)
    {
        m_lpCLogUtils = new CLogUtils();
    }
    return m_lpCLogUtils;
}