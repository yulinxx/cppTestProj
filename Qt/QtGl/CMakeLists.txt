cmake_minimum_required(VERSION 3.10)

MESSAGE(" --------- QtGL ----- \n")
project(QtGL)

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

set(CPP_DIR
    ./
)

# Boost支持配置
set(Boost_USE_STATIC_LIBS ON)  # 使用静态库
set(Boost_USE_MULTITHREADED ON)  # 启用多线程支持
set(Boost_USE_STATIC_RUNTIME OFF)  # 不使用静态运行时
find_package(Boost REQUIRED)  # 查找Boost

#################### EXE ##################################
foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME QtOpenGL_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        find_package(GLEW REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE GLEW::GLEW)

        find_package(glfw3 CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glfw)

        find_package(glad CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glad::glad)

        find_package(Stb REQUIRED)
        target_include_directories(${APP_NAME} PRIVATE ${Stb_INCLUDE_DIR})

        find_package(Qt5 COMPONENTS Widgets REQUIRED)
        include_directories(${Qt5Widgets_INCLUDE_DIRS})

        target_include_directories(${APP_NAME} PUBLIC ./)
        target_link_libraries(${APP_NAME} PUBLIC Qt5::Core Qt5::Widgets)


        if(Boost_FOUND)
            # message(STATUS "Boost found: ${Boost_VERSION}")
            # message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
            # message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
            # 将Boost的include目录添加到全局包含目录中
            include_directories(${Boost_INCLUDE_DIRS})
            # 为子项目提供Boost_FOUND变量
            set(Boost_FOUND ${Boost_FOUND} PARENT_SCOPE)
            set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} PARENT_SCOPE)
            set(Boost_LIBRARIES ${Boost_LIBRARIES} PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Boost not found. Please install Boost or check your configuration.")
        endif()

    endforeach(CPP)

endforeach(SUB_DIR)


# 添加 Windows 平台部署 Qt 依赖库的命令
if(WIN32)
    # 为每个创建的可执行文件设置 Windows 特定属性和部署命令
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        set(APP_NAME QtOpenGL_${FILE_NAME})
        
        # Windows 特定设置
        set_target_properties(${APP_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND "${Qt5_DIR}/../../../bin/windeployqt.exe" "$<TARGET_FILE:${APP_NAME}>"
            COMMENT "Deploying Qt libraries for ${APP_NAME}"
        )
    endforeach(CPP)
endif()