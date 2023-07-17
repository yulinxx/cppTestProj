MESSAGE(" --------- MathTestProj----- \n")

add_subdirectory(Eigen)
add_subdirectory(CGAL)
add_subdirectory(OpenCASCADE)

# # 从 CMakeLists.txt 中查找所有一级子目录并将它们添加到 add_subdirectory 中

# MESSAGE(" --------- --------------------------------------- ----- \n")

# 获取当前目录下的所有文件和子目录列表
# file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

# # 遍历列表
# foreach(child ${children})
#   # 检查是否为目录
#   if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
#     # 将子目录添加到 add_subdirectory 中
#     add_subdirectory(${child})
#     message(${child})
#   endif()
# endforeach()

# file(GLOB_RECURSIVE children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
# file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
# foreach(child ${children})
#   if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
#     message(${child})
#     add_subdirectory(${child}}
#     endif()
# endforeach()


# MESSAGE(" --------- --------------------------------------- ----- \n")
# MESSAGE(" --------- --------------------------------------- ----- \n")
# MESSAGE(" --------- --------------------------------------- ----- \n")
# foreach(SUBDIR ${SUBDIRECTORIES})

#   # 将子目录添加到 add_subdirectory 中
#   add_subdirectory(${SUBDIR})

# endforeach()