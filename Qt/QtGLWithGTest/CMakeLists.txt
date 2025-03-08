cmake_minimum_required(VERSION 3.10)

if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})

MESSAGE(" --------- QtWithGTest ----- \n")
project(QtWithGTest)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 根据不同操作系统设置Qt的CMake前缀路径
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH C:/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# 查找 GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(EXE_NAME QtWithGTest_Main)

# 主程序目标
add_executable(${EXE_NAME} src/main.cpp src/OpenGLWidget.cpp)
target_include_directories(${EXE_NAME} PUBLIC ./)
target_link_libraries(${EXE_NAME} PUBLIC Qt5::Core Qt5::Widgets)

# 添加测试目录
add_subdirectory(tests)

# # 测试目标
# add_executable(tests tests/OpenGLWidgetTest.cpp src/OpenGLWidget.cpp)
# target_include_directories(tests PUBLIC ./)
# target_link_libraries(tests PUBLIC Qt5::Core Qt5::Widgets GTest::GTest GTest::Main)