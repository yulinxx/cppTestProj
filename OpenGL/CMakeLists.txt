# OpenGL目录配置
MESSAGE(" ---------  OpenGL ----- \n")
# 自定义宏，用于查找当前目录下的所有子目录
macro(subdirlist result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(FIND EXCLUDE_DIRS ${child} _index)
            if(_index EQUAL -1)
                list(APPEND dirlist ${child})
            endif()
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

# 定义要排除的目录列表
set(EXCLUDE_DIRS ".git" ".vscode" "build")

# 查找并添加子目录
subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
    message(STATUS "Adding OpenGL subdirectory: ${subdir}")
    add_subdirectory(${subdir})
endforeach()