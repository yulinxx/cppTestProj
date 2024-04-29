cmake_minimum_required(VERSION 3.10)
project(InterfaceProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5包
find_package(Qt5 COMPONENTS  Core Widgets  REQUIRED)
# 设置包含Qt5头文件的目录
include_directories(${Qt5Widgets_INCLUDE_DIRS})

qt5_wrap_cpp(MOC_SOURCES IAboutWidget.h)

set(LIB_NAME LibExportInterface)
add_library(${LIB_NAME} SHARED Interface.cpp IAboutWidget.cpp ${MOC_SOURCES})

target_include_directories(${LIB_NAME}  PUBLIC ./)

target_compile_definitions(${LIB_NAME}  PUBLIC EXPORT_LIB)

target_link_libraries(${LIB_NAME}  PUBLIC  Qt5::Core Qt5::Widgets)
