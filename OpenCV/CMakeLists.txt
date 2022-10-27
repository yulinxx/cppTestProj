message(" ---------  OpenCV ----- \n")

find_package(OpenCV REQUIRED)

if(OpenCV_found)
    message(status "------Opencv Found")
    message(status ${OpenCV_INCLUDE_DIRS})
    message(status ${OpenCV_LIBRARY_DIRS})
    message(status ${OpenCV_LIBS})
    
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
elseif()
    message(FATAL_ERROR "------Opencv Not Found")
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
message("\ncur dir----------------------------" ${SUBDIR} )

    SET(ADD_DIR TRUE)
    foreach(exc ${ExclusionList})
        if("${SUBDIR}" STREQUAL "${exc}" )
            message(${SUBDIR}  "   Equal  " ${exc})
            SET(ADD_DIR FALSE)
            BREAK()
        endif()
    endforeach()

    IF( ADD_DIR )
        message("add subdir: " ${SUBDIR})
        ADD_SUBDIRECTORY(${SUBDIR})
    ENDIF()

ENDFOREACH()