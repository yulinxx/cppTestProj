# Cpp/base目录配置
MESSAGE(" --------- base ----- \n")
set(CPP_DIR ./)

foreach(sub_dir ${CPP_DIR})
    file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/${sub_dir}/*.cpp")
    foreach(cpp_file ${src_files})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${cpp_file})
        # message(STATUS "Building executable: base_${file_name}")
        add_executable(base_${file_name} ${cpp_file})
    endforeach()
endforeach()

