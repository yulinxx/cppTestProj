#ifndef _TINYCAD_LOG_H_
#define _TINYCAD_LOG_H_

// log
// #define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#define WRITE_INFOLOG(s) (LOG(INFO) << (s))    // 输出一个Info日志
#define WRITE_WARNLOG(s) (LOG(WARNING) << (s)) // 输出一个Warning日志
#define WRITE_ERRORLOG(s) (LOG(ERROR) << (s))  // 输出一个Error日志
#define WRITE_FATALLOG(s) (LOG(FATAL) << (s))  // 输出一个Fatal日志，这是最严重的日志并且输出之后会中止程序

// LOG_IF(INFO, num > 10) << "Got lots of cookies";  // 当条件满足时输出日志

void initLog(std::string &strName)
{
    // log
    FLAGS_log_dir = "./";   // 日志文件保存目录，这个目录必须是已经存在的
    FLAGS_max_log_size = 1; //设置最大日志文件大小（以MB为单位）
    FLAGS_logtostderr = false;
    // FLAGS_colorlogtostderr = true;  // Set log color
    FLAGS_stop_logging_if_full_disk = true; // If disk is full

    // FLAGS_servitysinglelog = true;// 用来按照等级区分log文件</span>

    google::SetLogDestination(google::GLOG_FATAL, "FATAL");     // 设置 GOOGLE_NAMESPACE::FATAL 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_ERROR, "ERROR");     // 设置 GOOGLE_NAMESPACE::ERROR 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_WARNING, "WARNING"); // 设置 GOOGLE_NAMESPACE::WARNING 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_INFO, "INFO");       // 设置 GOOGLE_NAMESPACE::INFO 级别的日志存储路径和文件名前缀
    // google::SetLogDestination(google::INFO, "/var/tmp/another_destination.INFO");

    FLAGS_timestamp_in_logfile_name = false;

    FLAGS_logbufsecs = 0; //缓冲日志输出，默认为30秒，此处改为立即输出

    google::SetLogFilenameExtension(".log");

    google::InitGoogleLogging(strName.c_str());

    google::FlushLogFiles(google::GLOG_INFO);
}

void closeLog()
{
    google::ShutdownGoogleLogging();
}

// class log
// {
// public:
//     log()
//     {

//     }
// public:
//     void initLog()
//     {

//     }
// };

#endif // _TINYCAD_LOG_H_