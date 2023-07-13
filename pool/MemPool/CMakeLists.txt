
MESSAGE(" ---------  MemPool ----- \n")

set(CPP_DIR
    ./
)


add_executable("MemPoolTest" main.cpp)

# foreach(SUB_DIR ${CPP_DIR})
#     file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
#     foreach(CPP ${SRC})
#         STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

#         set(EXE_NAME MemPool_${FILE_NAME})
#         add_executable(${EXE_NAME} ${CPP})

#     endforeach(CPP)    

# endforeach(SUB_DIR)
