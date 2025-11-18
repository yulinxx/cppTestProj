# Qt目录配置

message(STATUS "---------Configuring Qt projects---------")

# 定义要排除的目录列表
set(EXCLUDE_DIRS ".git" ".vscode" "build")

# 获取当前目录下的所有文件和子目录列表
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

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

