cmake_minimum_required(VERSION 3.10)

# 使用vcpkg进行包管理
if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
endif(WIN32)

set(CMAKE_TOOLCHAIN_FILE  ${VCPKG_PATH})

# set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(cppTest VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)


if(WIN32)
    # set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
    list(APPEND CMAKE_PREFIX_PATH C:/Install/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    # set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
endif(WIN32)


# 查找所有子目录
# 查找所有子目录并排除某些目录
MESSAGE("--------- subdir ----- ")

# 定义要排除的目录列表
SET(EXCLUDE_DIRS ".vs" ".vscode" ".git" "out" )

MACRO(SUBDIRLIST result curdir)
    # 获取当前目录下的所有子项
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)

    # 初始化目录列表
    SET(dirlist "")

    # 遍历所有子项
    FOREACH(child ${children})
        # 检查是否是目录
        IF(IS_DIRECTORY ${curdir}/${child})
            # 检查是否在排除列表中
            LIST(FIND EXCLUDE_DIRS ${child} _index)
            IF(_index EQUAL -1)  # 如果不在排除列表中
                LIST(APPEND dirlist ${child})
            ENDIF()
        ENDIF()
    ENDFOREACH()

    # 设置最终的目录列表
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
