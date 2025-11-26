# Qt目录配置

message(STATUS "---------Configuring Qt projects---------")

# 定义要排除的目录列表
set(EXCLUDE_DIRS ".git" ".vscode" "build")

# 获取当前目录下的所有文件和子目录列表
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

# 根据不同操作系统设置Qt的CMake前缀路径
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH C:/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
endif()

# 遍历列表并添加子目录
foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
    list(FIND EXCLUDE_DIRS ${child} _index)
    if(_index EQUAL -1)
      # message(STATUS "Adding Qt subdirectory: ${child}")
      add_subdirectory(${child})
    endif()
  endif()
endforeach()

