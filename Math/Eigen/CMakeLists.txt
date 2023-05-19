MESSAGE(" --------- EigenTestProj----- \n")


######################################################
set(CPP_DIR
    ./
)

find_package(Eigen3 CONFIG REQUIRED)


if(Eigen3_FOUND)
    MESSAGE( "--------------Eigen3_FOUND Found")
    MESSAGE( " ---Eigen3_FOUND Include: ${Eigen3_INCLUDE_DIRS}")
    MESSAGE( " ---Eigen3_FOUND Library: ${Eigen3_LIBRARY_DIRS}")

    foreach(lib ${Eigen3_LIBS})
       MESSAGE(" ${lib}")
    endforeach(lib )
    
    include_directories(${Eigen3_INCLUDE_DIRS})
    link_directories(${Eigen3_LIBRARY_DIRS})
elseif(Eigen3_FOUND)
    MESSAGE(FATAL_ERROR "------Eigen3 Not Found")
endif(Eigen3_FOUND)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        add_executable(Eigen3_${FILE_NAME} ${CPP})

        target_link_libraries(Eigen3_${FILE_NAME} PRIVATE Eigen3::Eigen)
    endforeach(CPP)    

endforeach(SUB_DIR)

