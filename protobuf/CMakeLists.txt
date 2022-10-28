
############################# Protobuf #############################
message(" --------- protobuf ----- \n")

find_package(Protobuf REQUIRED)
if(Protobuf_FOUND)

elseif()
    message(FATAL_ERROR "------Protobuf Not Found")
endif()

add_definitions(-DPROTOBUF_USE_DLLS)

if (Protobuf_FOUND)
    MESSAGE("1 PROTOBUF FOUNED")
    # C:/Install/vcpkg/installed/x86-windows/include
    MESSAGE("2 PROTOBUF_INCLUDE_DIR: ${PROTOBUF_INCLUDE_DIR}\n")
    # optimized;C:/Install/vcpkg/installed/x86-windows/lib/libprotobuf.lib;
    #     debug;C:/Install/vcpkg/installed/x86-windows/debug/lib/libprotobufd.lib
    MESSAGE("3 PROTOBUF_LIBRARIES: ${PROTOBUF_LIBRARIES}\n")
    # C:/Install/vcpkg/installed/x86-windows/debug/lib/libprotobufd.lib
    MESSAGE("4 PROTOBUF_LIBRARY_DEBUG: ${PROTOBUF_LIBRARY_DEBUG}\n")

    INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
else (Protobuf_FOUND)
    MESSAGE(FATAL_ERROR "------PROTOBUF NOT FOUNED")
endif (Protobuf_FOUND)


include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS MESSAGE.proto)
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ./proto/MESSAGE.proto)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})

MESSAGE("5 " ${PROTO_SRCS} ${PROTO_HDRS})
MESSAGE("6 " ${Protobuf_LIBRARIES})

####################################################################

set(CPP_DIR
    ./
)

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(EXE_NAME protobuf_${FILE_NAME})
        add_executable(${EXE_NAME} ${CPP} ${PROTO_SRCS})

        target_include_directories(${EXE_NAME} PRIVATE ./)
        target_link_libraries(${EXE_NAME} ${PROTOBUF_LIBRARY_DEBUG}) 
    endforeach(CPP)    

endforeach(SUB_DIR)
