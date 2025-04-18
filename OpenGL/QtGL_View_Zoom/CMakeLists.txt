cmake_minimum_required(VERSION 3.5)

##################################################################
# set(VCPKG_DIR "D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake")
#set(VCPKG_DIR "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(Qt_INSTALL_DIR "C:/Qt/5.15.2/msvc2019_64/")
set(Qt_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")
# set(Qt_DIR "C:/Qt/5.15.2/msvc2019_64/")

#set(OUT_CMAKE_MSG  TRUE)
set(OUT_CMAKE_MSG FALSE)


project(GLViewZoom LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(Qt5_DIR ${Qt_DIR})

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/viewwrapper.cpp
  src/marchview.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::OpenGL)

# 添加 Windows 平台部署 Qt 依赖库的命令
if(WIN32)
    # Windows 特定设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${Qt_INSTALL_DIR}/bin/windeployqt.exe" "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt libraries for Windows"
    )
endif()