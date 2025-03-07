MESSAGE(" --------- VTK Base ----- \n")

#################### Exe ##################################

set(CPP_DIR
    ./
)


# 查找 VTK 包
find_package(VTK REQUIRED)
if (NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found!")
endif()

# 输出 VTK 信息以供调试
message(STATUS "VTK Version: ${VTK_VERSION}")
message(STATUS "VTK Include Dirs: ${VTK_INCLUDE_DIRS}")
message(STATUS "VTK Library Dirs: ${VTK_LIBRARY_DIRS}")

# 包含 VTK 头文件
include_directories(${VTK_INCLUDE_DIRS})

# 如果使用 VTK 的 CMake 模块
include(${VTK_USE_FILE})

#################### EXE ##################################

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME VTK_Base_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        # 链接 VTK 库
        target_link_libraries(${APP_NAME} PRIVATE ${VTK_LIBRARIES})
    endforeach(CPP)
endforeach(SUB_DIR)