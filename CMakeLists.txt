cmake_minimum_required(VERSION 3.0.0)

# 使用vcpkg进行包管理
if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
endif(WIN32)

set(CMAKE_TOOLCHAIN_FILE  ${VCPKG_PATH})

# set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(cppTest VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

# 查找所有子目录 
MESSAGE("--------- subdir ----- ")
MACRO(SUBDIRLIST result curdir)

    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)

    SET(dirlist "")

    FOREACH(child ${children})

        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    
    SET(${result} ${dirlist})
ENDMACRO()

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

MESSAGE("--------- all done ----- ")
