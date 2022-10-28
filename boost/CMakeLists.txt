message(" --------- boostTestProj----- \n")

set(CPP_DIR
    ./src
    ./src/serialization
    ./src/StateMachine
    ./src/k-dimensional_tree
    ./src/geometry
)

message(STATUS "__CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "__PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})

foreach(SUB_DIR ${CPP_DIR})
    message(STATUS "---Sub Dir: ${SUB_DIR}")

    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    message(STATUS "---all cpp: ${SRC}")

    foreach(CPP ${SRC})
        message(STATUS "---Create Exe From: ${CPP}")

        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        MESSAGE(" --- FILE_NAME : "  ${FILE_NAME})

        add_executable(BOOST_${FILE_NAME} ${CPP})

    endforeach(CPP)    

endforeach(SUB_DIR)

message(" --------- Files Test B----- \n")

######################################################

find_package(Boost REQUIRED)

if(Boost_FOUND)
    message(status "--------------BOOST Found")
    message(status " ---BOOST Include: ${Boost_INCLUDE_DIRS}")
    message(status " ---BOOST Library: ${Boost_LIBRARY_DIRS}")

    foreach(lib ${Boost_LIBS})
       message(STATUS " ${lib}")
    endforeach(lib )
    
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
elseif(Boost_FOUND)
    message(FATAL_ERROR "------Boost Not Found")
endif(Boost_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

