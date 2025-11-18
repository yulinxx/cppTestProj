MESSAGE(" --------- VTK Base ----- \n")

# 设置CMP0074政策，使find_package使用<PackageName>_ROOT变量
cmake_policy(SET CMP0074 NEW)

#################### Exe ##################################

set(CPP_DIR
    ./
)


# 查找 VTK 包，指定所需组件以减少不必要的依赖加载
find_package(VTK REQUIRED COMPONENTS CommonCore RenderingCore)
if (NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found!")
endif()

# 输出 VTK 信息以供调试
# message(STATUS "VTK Version: ${VTK_VERSION}")
# message(STATUS "VTK Include Dirs: ${VTK_INCLUDE_DIRS}")
# message(STATUS "VTK Library Dirs: ${VTK_LIBRARY_DIRS}")

# 包含 VTK 头文件
include_directories(${VTK_INCLUDE_DIRS})

# 注意：在VTK 8.90及以上版本中，不再需要include(VTK_USE_FILE)
# 现代VTK版本推荐直接通过target_link_libraries链接所需组件

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