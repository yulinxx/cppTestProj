MESSAGE(" --------- template ----- \n")

set(CPP_DIR
    ./
)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        add_executable(template_${FILE_NAME} ${CPP})

    endforeach(CPP)    

endforeach(SUB_DIR)

