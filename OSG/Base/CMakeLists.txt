MESSAGE(" --------- OSG Base ----- \n")

#################### Exe ##################################

set(CPP_DIR
    ./
)

#set(CMAKE_PREFIX_PATH "D:/Install/vcpkg/installed/x64-windows")

# 查找 OpenSceneGraph
find_package(OpenSceneGraph REQUIRED COMPONENTS osgDB osgGA osgUtil osgViewer)
if(OpenSceneGraph_FOUND)
    #include_directories(${OpenSceneGraph_INCLUDE_DIRS})
    include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

    message(STATUS "Found OpenSceneGraph: ${OpenSceneGraph_VERSION}")
    message(STATUS "Include dirs: ${OpenSceneGraph_INCLUDE_DIRS}")
    message(STATUS "Libraries: ${OPENSCENEGRAPH_LIBRARIES}")
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
        #target_include_directories(${APP_NAME} PRIVATE ${OSG_INCLUDE_DIRS})

        # 链接 OSG 的库
        target_link_libraries(${APP_NAME} PRIVATE ${OpenSceneGraph_LIBRARIES} ${OPENSCENEGRAPH_LIBRARIES})
        #target_link_libraries(${APP_NAME} PRIVATE ${OSG_LIBRARIES})
    endforeach(CPP)
endforeach(SUB_DIR)