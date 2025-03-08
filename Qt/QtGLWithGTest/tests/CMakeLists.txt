cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(GTest_Unit)

# 启用C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 查找Qt5 Widgets和GTest
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})

set(app_name QtWithGTest_Test)
add_executable(${app_name} OpenGLWidgetTest.cpp ../src/OpenGLWidget.cpp)

# 链接Qt5 Widgets和GTest库
target_link_libraries(${app_name} PUBLIC Qt5::Core Qt5::Widgets GTest::GTest GTest::Main)



# add_executable(tests tests/OpenGLWidgetTest.cpp src/OpenGLWidget.cpp)
# target_include_directories(tests PUBLIC ./)
# target_link_libraries(tests PUBLIC Qt5::Core Qt5::Widgets GTest::GTest GTest::Main)


# 添加自定义命令来运行测试
# add_custom_target(RunTests COMMAND tests_gtest)
# add_custom_target(RunTestsMain COMMAND ${app_name})