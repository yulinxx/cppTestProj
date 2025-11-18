
# Protobuf目录配置

message(STATUS "---------Configuring Protobuf project---------")

# 查找Protobuf库
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found")
endif()

add_definitions(-DPROTOBUF_USE_DLLS)

# 设置包含目录
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 生成proto文件
file(GLOB proto_files "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_files})

# 构建可执行文件
set(CPP_DIR ./)

foreach(sub_dir ${CPP_DIR})
    file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/${sub_dir}/*.cpp")
    foreach(cpp_file ${src_files})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${cpp_file})
        set(exe_name protobuf_${file_name})
        # message(STATUS "Building Protobuf executable: ${exe_name}")
        
        add_executable(${exe_name} ${cpp_file} ${PROTO_SRCS})
        target_include_directories(${exe_name} PRIVATE ./)
        target_link_libraries(${exe_name} ${PROTOBUF_LIBRARY_DEBUG})
    endforeach()
endforeach()
