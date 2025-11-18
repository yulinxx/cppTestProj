# Skia目录配置

message(STATUS "---------Configuring Skia project---------")

# 设置Qt路径
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH C:/Install/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
endif()

# 查找Qt组件
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 not found")
endif()

# 设置编译选项
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Skia库
find_package(unofficial-skia CONFIG REQUIRED)
if(NOT unofficial-skia_FOUND)
    message(FATAL_ERROR "unofficial-skia not found")
endif()

message(STATUS "Skia found. Include dirs: ${unofficial-skia_INCLUDE_DIRS}")

# 设置包含目录
include_directories(${unofficial-skia_INCLUDE_DIRS})
link_directories(${unofficial-skia_LIBRARY_DIRS})

# 设置源文件
set(PROJECT_SOURCES
    main.cpp
    SkiaCircleWidget.cpp
    SkiaCircleWidget.h
    # test/mainwindow.ui
    # ${TS_FILES}
)

# 创建可执行文件
set(APP_NAME SkiaTestProj)
# message(STATUS "Building Skia executable: ${APP_NAME}")

add_executable(${APP_NAME} ${PROJECT_SOURCES})

# 链接库
qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${APP_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(${APP_NAME} PRIVATE unofficial::skia::skia)