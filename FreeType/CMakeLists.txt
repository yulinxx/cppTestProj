cmake_minimum_required(VERSION 3.10)

# 项目名称
project(FreeTypeExample)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)


# 查找 FreeType 包
find_package(Freetype REQUIRED)

# 添加源文件
add_executable(FreeTypeExample base.cpp)

# 链接 FreeType 库
target_link_libraries(FreeTypeExample PRIVATE Freetype::Freetype)
