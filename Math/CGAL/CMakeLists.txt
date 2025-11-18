MESSAGE(" --------- CGAL TestProj----- \n")


######################################################
set(CPP_DIR
    ./
)

find_package(CGAL CONFIG REQUIRED) 

if(CGAL_FOUND)
    # MESSAGE( "--------------CGAL_FOUND Found")
    # MESSAGE( " ---CGAL_FOUND Include: ${CGAL_INCLUDE_DIRS}")
    # MESSAGE( " ---CGAL_FOUND Library: ${CGAL_LIBRARY_DIRS}")

    # foreach(lib ${CGAL_LIBS})
    #    MESSAGE(" ${lib}")
    # endforeach(lib )
    
    include_directories(${CGAL_INCLUDE_DIRS})
    link_directories(${CGAL_LIBRARY_DIRS})

else()
    MESSAGE(FATAL_ERROR "------CGAL Not Found")
endif(CGAL_FOUND)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        add_executable(CGAL_${FILE_NAME} ${CPP})

        target_link_libraries(CGAL_${FILE_NAME} PRIVATE CGAL::CGAL)
    endforeach(CPP)    

endforeach(SUB_DIR)

