cmake_minimum_required(VERSION 3.10)
project(ImplementationProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 查找Qt5包
find_package(Qt5 COMPONENTS  Core Widgets  REQUIRED)

# 设置包含Qt5头文件的目录
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

qt5_wrap_cpp(MOC_SOURCES RdAboutWidget.h)

set(LIB_NAME LibExportImplementation)
add_library(${LIB_NAME} SHARED
        Implementation.cpp RdAboutWidget.cpp
        ${MOC_SOURCES}
    )

target_include_directories(${LIB_NAME} PUBLIC ../LibExportInterface/)
target_link_libraries(${LIB_NAME} PUBLIC LibExportInterface)
target_link_libraries(${LIB_NAME} PUBLIC  Qt5::Core Qt5::Widgets)

