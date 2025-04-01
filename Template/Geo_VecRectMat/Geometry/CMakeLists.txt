cmake_minimum_required(VERSION 3.10)
project(Geometry)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED 
./include/Vec.h ./src/Vec.cpp
./include/Rect.h ./src/Rect.cpp
./include/Matrix.h ./src/Matrix.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE UTILITY_EXPORTS)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
