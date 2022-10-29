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

void OnClientDumpRequestCallabck(void *context, const ClientInfo *client_info, const string *file_path)
{
}

int main(void)
{
    CrashGenerationServer::CreateReportChannel(&server_fd, &client_fd);

    CrashGenerationServer crash_server(
        server_fd,
        &OnClientDumpReuqestCallabck,
        NULL,
        NULL,
        NULL,
        true,
        "/tmp");
    crash_server.Start();

    // 拉起检查崩溃进程，并且传入client_fd, 在崩溃检查进程中初始化ExceptionHandler类，最后一个参数传入client_fd
}
