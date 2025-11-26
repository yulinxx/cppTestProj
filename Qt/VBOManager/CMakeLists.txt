cmake_minimum_required(VERSION 3.10)

MESSAGE(" --------- QtGLVBOManager ----- \n")
project(QtWithGTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加UTF-8编码支持
if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt5组件
find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS})

# 应用程序名称
set(APP_NAME QtGLVBOManager)

# 获取所有源文件和头文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")
file(GLOB_RECURSE RESOURCES "res/*.qrc")

# 主程序目标
add_executable(${APP_NAME} 
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# 设置包含目录
target_include_directories(${APP_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(${APP_NAME} 
    PUBLIC 
        Qt5::Core 
        Qt5::Widgets 
        Qt5::OpenGL
)

# 添加 Windows 平台部署 Qt 依赖库的命令
if(WIN32)
    # 设置Windows特定属性
    set_target_properties(${APP_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # 查找windeployqt工具
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS "${Qt5_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        # 添加部署命令
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${APP_NAME}>"
            COMMENT "Deploying Qt libraries for ${APP_NAME}"
        )
    else()
        message(WARNING "windeployqt not found. Qt libraries will not be deployed automatically.")
    endif()
endif()