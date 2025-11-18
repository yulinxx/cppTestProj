cmake_minimum_required(VERSION 3.10)

# 使用vcpkg进行包管理
if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})
# set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(cppTest VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# 根据不同操作系统设置Qt的CMake前缀路径
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH C:/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
    # 可在此处添加苹果系统的Qt路径设置
endif()

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
set(EXCLUDE_DIRS ".vs" ".vscode" ".git" "out" "build")

# 查找并添加子目录
subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
    # message(STATUS "Adding subdirectory: ${subdir}")
    add_subdirectory(${subdir})
endforeach()

message(STATUS "Project configuration completed.")
