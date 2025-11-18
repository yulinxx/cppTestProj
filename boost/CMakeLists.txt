# Boost目录配置

message(STATUS "---------Configuring Boost projects---------")

# 设置CMP0167策略以避免Boost模块警告
cmake_policy(SET CMP0167 NEW)

# 查找Boost库
find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

message(STATUS "Boost found. Include dirs: ${Boost_INCLUDE_DIRS}")

# 设置包含目录
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${Boost_LIBRARY_DIRS})

# 定义源目录
set(CPP_DIR
    ./src
    ./src/other
    ./src/serialization
    ./src/StateMachine
    ./src/k-dimensional_tree
    ./src/geometry
)

# 构建可执行文件
foreach(sub_dir ${CPP_DIR})
    file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/${sub_dir}/*.cpp")
    foreach(cpp_file ${src_files})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${cpp_file})
        set(exe_name BOOST_${file_name})
        message(STATUS "Building Boost executable: ${exe_name}")
        
        add_executable(${exe_name} ${cpp_file})
    endforeach()
endforeach()

