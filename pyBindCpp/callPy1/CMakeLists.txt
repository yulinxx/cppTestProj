# 查找Python库
find_package(Python3 COMPONENTS Development REQUIRED) 
if (Python3_FOUND)
    include_directories( ${Python3_INCLUDE_DIRS} )
    # add_definitions( ${Python3_DEFINITIONS} )
    # target_include_directories(addtest PRIVATE ${Python3__INCLUDE_DIR})
    # target_link_libraries( ${Python3_LIBRARY})
    
    include_directories(${Python3_INCLUDE_DIRS})
    link_directories(${Python3_LIBRARY_DIRS})
else(Python3_FOUND)
    message(FATAL_ERROR "Python3 library not found")
endif(Python3_FOUND)

# # 导入vcpkg安装的库集成文件
# list(APPEND CMAKE_PREFIX_PATH "C:/Install/vcpk/installed/x64-windows/share/")

# # 导入vcpkg安装的库集成文件
# list(APPEND CMAKE_PREFIX_PATH "C:/Install/vcpkg/installed/x64-windows/share/")
# include_directories("C:/Install/vcpkg/installed/x64-windows/include")

# 添加可执行文件
set(APP_NAME CallPy1)
add_executable(${APP_NAME} main.cpp)

# 设置 C++ 标准
set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# 包含 PyBind11 头文件
target_include_directories(${APP_NAME} PRIVATE ${pybind11_INCLUDE_DIRS})

# 链接 Python 库
target_link_libraries(${APP_NAME} PRIVATE ${Python3_LIBRARIES})

target_link_libraries(${APP_NAME} PRIVATE ${pybind11_LIBRARIES})
