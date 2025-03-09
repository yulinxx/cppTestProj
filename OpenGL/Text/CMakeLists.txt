# 设置 vcpkg 的工具链文件
#set(CMAKE_TOOLCHAIN_FILE "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
# 项目配置
project(OpenGL_Text)

# 添加源代码目录
set(CPP_DIR
./  # 设置源代码目录
)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(GLEW REQUIRED)

# 查找 FreeType 包
find_package(Freetype REQUIRED)

find_package(freetype-gl CONFIG REQUIRED)

# 遍历源代码目录并创建目标
foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME OpenGL_Text_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        # 添加 freetype-gl 的包含目录
        target_include_directories(${APP_NAME} PRIVATE ${freetype-gl_INCLUDE_DIRS})

        # 链接 OpenGL 和其他库
        target_link_libraries(${APP_NAME} PRIVATE glfw)
        target_link_libraries(${APP_NAME} PRIVATE glad::glad)

        # 链接 FreeType 库
        target_link_libraries(${APP_NAME} PRIVATE Freetype::Freetype)
        target_link_libraries(${APP_NAME} PRIVATE freetype-gl)
        target_link_libraries(${APP_NAME} PRIVATE GLEW::GLEW)

    endforeach()
endforeach()