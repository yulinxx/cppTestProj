cmake_minimum_required(VERSION 3.10)
project(GeoExe)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 链接VecLib库
target_link_libraries(${PROJECT_NAME} PRIVATE Geometry)

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/VecLib/include)