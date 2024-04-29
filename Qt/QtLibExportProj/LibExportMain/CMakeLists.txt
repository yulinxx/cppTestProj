cmake_minimum_required(VERSION 3.10)
project(LibUser)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5包
find_package(Qt5 COMPONENTS Widgets REQUIRED)
# 设置包含Qt5头文件的目录
include_directories(${Qt5Widgets_INCLUDE_DIRS})

set(LIB_NAME LibExportMain)

add_executable(${LIB_NAME} main.cpp MainWindow.cpp)

target_include_directories(${LIB_NAME} PUBLIC ./)
target_include_directories(${LIB_NAME} PUBLIC ../LibExportImplementation/)

#target_link_libraries(${LIB_NAME} PUBLIC Interface)
target_link_libraries(${LIB_NAME} PUBLIC LibExportImplementation)


target_link_libraries(${LIB_NAME} PUBLIC  Qt5::Core Qt5::Widgets)
