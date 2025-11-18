MESSAGE(" ---------  OpenCV ----- \n")

# 解决CMP0074政策警告 - 使find_package使用<PackageName>_ROOT变量
cmake_policy(SET CMP0074 NEW)

find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    MESSAGE( "------Opencv Found")
    MESSAGE( "${OpenCV_INCLUDE_DIRS}")
    MESSAGE( "${OpenCV_LIBRARY_DIRS}")
    MESSAGE( "${OpenCV_LIBS}")
    
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
else()
    MESSAGE(FATAL_ERROR "------Opencv Not Found")
endif()


# 查找当前文件夹下的子目录
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}) 

# 设置要排除的子目录
SET(ExclusionList
    .git
    .vscode
    build
)

# 遍历子目录,将排除的子目剔除后,添加至cmake中
FOREACH(SUBDIR ${SUBDIRS})
MESSAGE("\n----------------------------" ${SUBDIR} )

    SET(ADD_DIR TRUE)
    foreach(exc ${ExclusionList})
        if("${SUBDIR}" STREQUAL "${exc}" )
            MESSAGE(" ${SUBDIR}   Equal  ${exc} Skip...")
            SET(ADD_DIR FALSE)
            BREAK()
        endif()
    endforeach()

    IF( ADD_DIR )
        MESSAGE("add subdir: " ${SUBDIR})
        ADD_SUBDIRECTORY(${SUBDIR})
    ENDIF()

ENDFOREACH()