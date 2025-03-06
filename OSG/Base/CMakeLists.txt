MESSAGE(" --------- OSG Base ----- \n")

#################### Exe ##################################

set(CPP_DIR
    ./
)

# 查找 OpenSceneGraph
find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgUtil)
if(OpenSceneGraph_FOUND)
    message(STATUS "Found OpenSceneGraph: ${OpenSceneGraph_VERSION}")
    message(STATUS "Include dirs: ${OpenSceneGraph_INCLUDE_DIRS}")
    message(STATUS "Libraries: ${OpenSceneGraph_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSceneGraph not found!")
endif()

#################### EXE ##################################

foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})

        set(APP_NAME OSG_Base_${FILE_NAME})
        add_executable(${APP_NAME} ${CPP})

        # 添加 OSG 的头文件路径
        target_include_directories(${APP_NAME} PRIVATE ${OpenSceneGraph_INCLUDE_DIRS})
        # 链接 OSG 的库
        target_link_libraries(${APP_NAME} PRIVATE ${OpenSceneGraph_LIBRARIES})
    endforeach(CPP)
endforeach(SUB_DIR)