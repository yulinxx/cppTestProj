# breakpad
breakpad是google开发的一个跨平台C/C++ dump捕获开源库，崩溃文件使用微软的minidump格式存储，也支持发送这个dump文件到你的服务器，  
breakpad可以在程序崩溃时触发dump写入操作，也可以在没有触发dump时主动写dump文件。  

breakpad支持windows、linux、macos、android、ios等。  
目前已有Google Chrome, Firefox, Google Picasa, Camino, Google Earth等项目使用。  

链接：https://juejin.cn/post/6899070041074073614


## Minidump  
Minidump是微软开发的一种用于崩溃时记录的小存储器转储文件，它类似与linux下的core file，minidump中包含以下信息：  

进程装载的驱动程序和共享库列表，这个列表中包含了指定的名称和版本号  
进程中存在的线程列表。对于每个线程，小型转储包括处理器寄存器的状态和线程堆栈内存的内容。这些数据是未解释的字节流，因为 Breakpad 客户端通常没有可用于生成函数名称或行号，甚至标识堆栈帧边界的调试信息。  
已停止的处理器的上下文 (PRCB)  
已停止的进程的信息和内核上下文 (EPROCESS)  
有关收集转储的系统的其他信息：处理器和操作系统版本、转储的原因等。  

Breakpad 在所有平台上使用 Windows 小型转储文件，而不是传统核心文件，原因如下：  

核心文件可能非常大，因此无法通过网络将其发送到收集器进行处理。小型转储更小，因为它们被设计为这样使用。  
核心文件格式记录得很差。例如，Linux 标准基础不描述寄存器如何存储在段中。  
说服 Windows 计算机生成核心转储文件比说服其他计算机编写小型转储文件更难。  
它简化了 Breakpad 处理器，仅支持一种文件格式。  


在Windows下使用breakpad的方法很简单，只需要创建一个ExceptionHandler的类即可  

### 进程内抓取Dump文件
进程内抓取Dump文件是最简单的breakpad的用法。使用方法很简单：  

```cpp
const std::wstring s_strCrashDir = L"c:\dumps";

bool
InitBreakpad()
{
    google_breakpad::ExceptionHandler *pCrashHandler =
        new google_breakpad::ExceptionHandler(s_strCrashDir,
                onExceptionFilter,
                onMinidumpDumped,
                NULL,
                google_breakpad::ExceptionHandler::HANDLER_ALL,
                MiniDumpNormal,
                NULL,
                NULL);

    if(pCrashHandler == NULL) {
        return false;
    }

    return true;
}
```
### 3.2. 进程外抓取Dump文件  
使用进程外抓取Dump时，需要指定服务端和客户端，在服务端中需要创建CrashGenerationServer的实例，  
而在客户端中则只需要创建  ExceptionHandler即可。  
此外，如果服务端自己需要抓进程内的Dump，请将pipe的参数置为NULL。  

```cpp
const wchar_t s_pPipeName[] = L"\\.\pipe\breakpad\crash_handler_server";
const std::wstring s_strCrashDir = L"c:\dumps";

bool
InitBreakpad()
{
    google_breakpad::CrashGenerationServer *pCrashServer =
        new google_breakpad::CrashGenerationServer(s_pPipeName,
                NULL,
                onClientConnected,
                NULL,
                onClientDumpRequest,
                NULL,
                onClientExited,
                NULL,
                true,
                &s_strCrashDir);

    if(pCrashServer == NULL) {
        return false;
    }

    // 如果已经服务端已经启动了，此处启动会失败
    if(!pCrashServer->Start()) {
        delete pCrashServer;
        pCrashServer = NULL;
    }

    google_breakpad::ExceptionHandler *pCrashHandler =
        new google_breakpad::ExceptionHandler(s_strCrashDir,
                onExceptionFilter,
                onMinidumpDumped,
                NULL,
                google_breakpad::ExceptionHandler::HANDLER_ALL,
                MiniDumpNormal,
                (pCrashServer == NULL) ? s_pPipeName : NULL, // 如果是服务端，则直接使用进程内dump
                NULL);

    if(pCrashHandler == NULL) {
        return false;
    }

    return true;
}
```
** 使用注意 ** 
使用breakpad的时候，有两个地方需要注意：  

记得把breakpad的solution下的几个工程，包含到你开发的工程中，或者直接包含他们的lib。  
common：基础功能，包含一个对GUID的封装和http上传的类。  
exception_handler：用来捕获崩溃的类。  
crash_generation_server：breakpad的服务端，用来在产生崩溃时抓取dump。  
crash_generation_client：breakpad的客户端，用来捕获当前进程的崩溃。  
在初始化breakpad之前，记得先创建好dump文件的目录，不然breakpad服务端将不能正常的写dump，  
这会导致breakpad客户端在崩溃时无限等待服务端dump写完的消息，最后失去响应。   

Google Breakpad的使用_无名无奈的博客-CSDN博客_googlebreakpad使用
[https://blog.csdn.net/weixin_40332490/article/details/105328775](https://blog.csdn.net/weixin_40332490/article/details/105328775)

现在看看breakpad在Windows下到底是如何实现的呢？  

1. 代码结构  
在我们来看breakpad是如何实现其强大的功能之前，我们先来看一下他的代码结构吧。  

Google breakpad的源代码都在src的目录下，他分为如下几个文件夹：  
client：这下面包含了前台应用程序中捕捉dump的部分代码，里面按照平台分成各个子文件夹  
common：前台后台都会用到的部分基础代码，字符串转换，内存读写，md5等  
google_breakpad：breakpad中公共的头文件  
processor：用于在后台处理崩溃的核心代码  
testing：测试工程  
third_party：第三方库  
tools：一些小工具，用于处理dump文件和符号表  
