cmake_minimum_required(VERSION 3.0.0)


set(CMAKE_TOOLCHAIN_FILE D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
# set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(cppTest VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

# include(CTest)
# enable_testing()

# add_executable(cppTest main.cpp)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

add_subdirectory(template)
add_subdirectory(cpp11)
add_subdirectory(base)
add_subdirectory(log4cplus_test)
add_subdirectory(machine_status)
add_subdirectory(network)

message("--------- all done ----- ")