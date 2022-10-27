# 一、glog介绍  
Google Logging（glog）是一个实现应用程序级日志记录的C++98库。该库提供了基于C++风格的流和各种辅助宏的日志API。  

glog的日志按严重等级分为四级，由低到高分别是：  

- INFO（0级）  
- WARNING（1级）  
- ERROR（2级）  
- FATAL（3级）  
注意：高严重等级的日志，会出现在所有比它等级低的日志中。  
例如，FATAL的日志，不仅会出现在FATAL中，也会出现在ERROR、WARNING、INFO之中。  

# 二、glog的基础使用  
- 包含头文件： 
`#include "glog/logging.h"`  

- 用glog之前必须先初始化库，要生成日志文件只需在开始log之前调用一次：  
`google::InitGoogleLogging(argv[0]); //括号内是程序名`   

- 当要结束glog时必须关闭库，否则会内存溢出：  
`google::ShutdownGoogleLogging();`  

`FLAGS_log_dir = " "; //设置日志文件输出目录`   

## 注意事项  
- GLog在整个项目周期中只允许生成一个日志文件，也就是说，glog从初始化到注销整个过程，不允许再次调用glog的初始化或者注销；即只允许有一个实例化对象；  
- 在需要使用glog打印日志的cpp文件中包含头文件，注意：每个使用glog的cpp文件都要添加；  

原文链接：https://blog.csdn.net/weixin_44947987/article/details/126214261  