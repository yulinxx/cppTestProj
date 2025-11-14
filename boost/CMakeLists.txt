MESSAGE(" --------- boostTestProj----- \n")

set(CPP_DIR
    ./src
    ./src/other
    ./src/serialization
    ./src/StateMachine
    ./src/k-dimensional_tree
    ./src/geometry
)

MESSAGE("__CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE("__PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})

foreach(SUB_DIR ${CPP_DIR})
    MESSAGE("---Sub Dir: ${SUB_DIR}")

    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    MESSAGE("---all cpp: ${SRC}\n")

    foreach(CPP ${SRC})
    
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        # MESSAGE("---Create Exe From: ${CPP}")
        # MESSAGE("---file name: ${FILE_NAME} \n")

        add_executable(BOOST_${FILE_NAME} ${CPP})

    endforeach(CPP)    

endforeach(SUB_DIR)

MESSAGE(" --------- Files Test B----- \n")

######################################################

find_package(Boost REQUIRED)

if(Boost_FOUND)
    MESSAGE( "--------------BOOST Found")
    MESSAGE( " ---BOOST Include: ${Boost_INCLUDE_DIRS}")
    MESSAGE( " ---BOOST Library: ${Boost_LIBRARY_DIRS}")

    foreach(lib ${Boost_LIBS})
       MESSAGE(" ${lib}")
    endforeach(lib )
    
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    MESSAGE(FATAL_ERROR "------Boost Not Found")
endif(Boost_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

