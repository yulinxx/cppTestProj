######################################################
MESSAGE(" --------- OpenGL  LineType ----- \n")

#################### Exe ##################################

set(CPP_DIR
    ./
)

#################### EXE ##################################
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME OpenGL_LineType_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        find_package(GLEW REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE GLEW::GLEW)

        find_package(glfw3 CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glfw)

        find_package(glad CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glad::glad)

    endforeach(CPP)

endforeach(SUB_DIR)
