
# aux_source_directory(CPP_SRC ./)

# add_executable(${CPP_SRC} BreakPadDump)

MESSAGE(" --------- breakpad ----- \n")

find_package(unofficial-breakpad CONFIG REQUIRED)
if(unofficial-breakpad_FOUND)
    
else()
    MESSAGE(FATAL_ERROR "------breakpad Not Found")
endif()


set(CPP_DIR
    ./
)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(EXE_NAME DUMP_${FILE_NAME})
        add_executable(${EXE_NAME} ${CPP})

        # target_include_directories(${EXE_NAME} PRIVATE ./)
        # target_link_libraries(${EXE_NAME} ${PROTOBUF_LIBRARY_DEBUG}) 
        target_link_libraries(${EXE_NAME} PRIVATE unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client)

    endforeach(CPP)    

endforeach(SUB_DIR)

