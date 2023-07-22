# 添加可执行文件
add_executable(MyPyBind11Example my_module.cpp)

# 设置 C++ 标准
set_target_properties(MyPyBind11Example PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# 包含 PyBind11 头文件
target_include_directories(MyPyBind11Example PRIVATE ${pybind11_INCLUDE_DIRS})

# 链接 Python 库
target_link_libraries(MyPyBind11Example PRIVATE ${Python3_LIBRARIES})

target_link_libraries(MyPyBind11Example PRIVATE ${pybind11_LIBRARIES})
