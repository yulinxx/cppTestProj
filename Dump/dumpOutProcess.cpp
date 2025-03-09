// 代码为 Linux 版, 需转成Windows版

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "client/windows/handler/exception_handler.h"

#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#include "client/mac/handler/exception_handler.h"

#else
#include "client/linux/handler/exception_handler.h"
#endif

int server_fd;
int client_fd;

/**
 * @brief 客户端转储请求回调函数
 *
 * 当客户端发送转储请求时,此函数会被调用.
 *
 * @param context 上下文指针,可用于传递额外信息
 * @param client_info 客户端信息结构体指针
 * @param file_path 转储文件路径字符串指针
 */
 // void OnClientDumpRequestCallabck(void *context, const ClientInfo *client_info, const string *file_path)
 // {
 // }

 /**
  * @brief 程序入口函数
  *
  * 此函数是程序的起始点,负责初始化崩溃转储服务并启动.
  *
  * @return int 程序退出状态码,0表示正常退出
  */
int main(void)
{
    // 创建崩溃报告通道,获取服务器和客户端的文件描述符
    // CrashGenerationServer::CreateReportChannel(&server_fd, &client_fd);

    // 创建崩溃转储服务器实例
    // CrashGenerationServer crash_server(
    //     server_fd,  // 服务器文件描述符
    //     &OnClientDumpReuqestCallabck,  // 客户端转储请求回调函数
    //     NULL,  // 自定义数据指针
    //     NULL,  // 过滤器函数指针
    //     NULL,  // 处理完成回调函数指针
    //     true,  // 是否允许异步处理
    //     "/tmp"  // 转储文件存储路径
    // );
    // 启动崩溃转储服务器
    // crash_server.Start();

    // 拉起检查崩溃进程,并且传入client_fd, 在崩溃检查进程中初始化ExceptionHandler类,最后一个参数传入client_fd
    return 0;
}