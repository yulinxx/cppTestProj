######################################################
MESSAGE(" --------- OpenGL  DashLine ----- \n")

#################### Exe ##################################

MESSAGE(" --------- EigenTestProj----- \n")


######################################################
set(CPP_DIR
    ./
)

#################### EXE ##################################
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# add_executable(DashLine ${SRC_FILES})

# find_package(GLEW REQUIRED)
# target_link_libraries(DashLine PRIVATE GLEW::GLEW)

# find_package(glfw3 CONFIG REQUIRED)
# target_link_libraries(DashLine PRIVATE glfw)

# find_package(glad CONFIG REQUIRED)
# target_link_libraries(DashLine PRIVATE glad::glad)


# find_package(Eigen3 CONFIG REQUIRED)


# if(Eigen3_FOUND)
#     MESSAGE( "--------------Eigen3_FOUND Found")
#     MESSAGE( " ---Eigen3_FOUND Include: ${Eigen3_INCLUDE_DIRS}")
#     MESSAGE( " ---Eigen3_FOUND Library: ${Eigen3_LIBRARY_DIRS}")

#     foreach(lib ${Eigen3_LIBS})
#        MESSAGE(" ${lib}")
#     endforeach(lib )

#     include_directories(${Eigen3_INCLUDE_DIRS})
#     link_directories(${Eigen3_LIBRARY_DIRS})
# elseif(Eigen3_FOUND)
#     MESSAGE(FATAL_ERROR "------Eigen3 Not Found")
# endif(Eigen3_FOUND)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME OpenGLDashLine_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        find_package(GLEW REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE GLEW::GLEW)

        find_package(glfw3 CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glfw)

        find_package(glad CONFIG REQUIRED)
        target_link_libraries(${APP_NAME} PRIVATE glad::glad)


        # target_link_libraries(OpenGLDashLine_${FILE_NAME} PRIVATE Eigen3::Eigen)
    endforeach(CPP)

endforeach(SUB_DIR)





