cmake_minimum_required(VERSION 3.10)
project(LibTestProj)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOGEN_MOC_MACROS "Q_MOC_RUNNER")


# 前两个工程分别为 基类 、 实现类，第三个为使用的程序 
# 基类导出基本的C++类和Qt的类， 实现类分别实现了基类的类
# 即 libA 导出 给 libB用， 最终 libB 给 Exe 用
add_subdirectory(LibExportInterface)
add_subdirectory(LibExportImplementation)
add_subdirectory(LibExportMain)

