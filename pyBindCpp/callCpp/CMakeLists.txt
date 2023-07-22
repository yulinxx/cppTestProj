 
# add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
find_package(Python3 COMPONENTS Development REQUIRED) 
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(example example.cpp)

# 指定 Python 的 include 目录
include_directories(${Python3_INCLUDE_DIRS})

# 指定 Python 的库目录
link_directories(${Python3_LIBRARY_DIRS})
