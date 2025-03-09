MESSAGE(" --------- EigeOrthGL----- \n")

######################################################
set(CPP_DIR
    ./
)

# 查找 Eigen3 库
find_package(Eigen3 CONFIG REQUIRED)

# 查找 GLEW 库
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
endif()

# 查找 GLFW 库
find_package(glfw3 REQUIRED)

# 查找 OpenGL 库
find_package(OpenGL REQUIRED)

if(Eigen3_FOUND)
    MESSAGE( "--------------Eigen3 Found")
    MESSAGE( " ---Eigen3 Include: ${Eigen3_INCLUDE_DIRS}")
    MESSAGE( " ---Eigen3 Library: ${Eigen3_LIBRARY_DIRS}")

    foreach(lib ${Eigen3_LIBS})
       MESSAGE(" ${lib}")
    endforeach(lib)
    
    include_directories(${Eigen3_INCLUDE_DIRS})
    link_directories(${Eigen3_LIBRARY_DIRS})
else()
    MESSAGE(FATAL_ERROR "------Eigen3 Not Found")
endif()

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        add_executable(Eigen3_gl_${FILE_NAME} ${CPP})

        # 链接库
        target_link_libraries(Eigen3_gl_${FILE_NAME} PRIVATE 
            Eigen3::Eigen
            ${GLEW_LIBRARIES}
            glfw
            ${OPENGL_LIBRARIES}
        )
    endforeach(CPP)    
endforeach(SUB_DIR)

