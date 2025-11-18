cmake_minimum_required(VERSION 3.10)
project(OpenGL_Base3D)

######################################################
MESSAGE(" --------- OpenGL Base3D \n")

#################### Exe ##################################

set(CPP_DIR
    ./
)

# 查找必要的库
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# 添加GLM头文件（GLM通常是仅头文件的库）
find_package(glm CONFIG REQUIRED)

#################### EXE ##################################
foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME OpenGL_MousePickup_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        # 链接库
        target_link_libraries(${APP_NAME} PRIVATE glfw glad::glad)
        target_link_libraries(${APP_NAME} PRIVATE glm::glm)
    endforeach(CPP)
endforeach(SUB_DIR)
