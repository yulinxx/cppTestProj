# OpenGL/Base目录配置
MESSAGE(" --------- OpenGL  Base 4 ----- \n")
set(CPP_DIR ./)

foreach(sub_dir ${CPP_DIR})
    file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/${sub_dir}/*.cpp")
    foreach(cpp_file ${src_files})
        string(REGEX REPLACE ".+/(.+)\\..*" "\\1" file_name ${cpp_file})
        set(app_name OpenGL4_Base_${file_name})
        # message(STATUS "Building OpenGL executable: ${app_name}")
        
        add_executable(${app_name} ${cpp_file})

        # 查找并链接必要的OpenGL库
        find_package(GLEW REQUIRED)
        target_link_libraries(${app_name} PRIVATE GLEW::GLEW)

        find_package(glfw3 CONFIG REQUIRED)
        target_link_libraries(${app_name} PRIVATE glfw)

        find_package(glad CONFIG REQUIRED)
        target_link_libraries(${app_name} PRIVATE glad::glad)

        find_package(Stb REQUIRED)
        target_include_directories(${app_name} PRIVATE ${Stb_INCLUDE_DIR})
    endforeach()
endforeach()
