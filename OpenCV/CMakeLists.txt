MESSAGE(" ---------  OpenCV ----- \n")

find_package(OpenCV REQUIRED)

if(OpenCV_found)
    MESSAGE(status "------Opencv Found")
    MESSAGE(status ${OpenCV_INCLUDE_DIRS})
    MESSAGE(status ${OpenCV_LIBRARY_DIRS})
    MESSAGE(status ${OpenCV_LIBS})
    
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
elseif()
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