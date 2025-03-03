# find_package(Python3 COMPONENTS Development REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11 CONFIG REQUIRED)

# add_subdirectory(callCpp)
add_subdirectory(callPy1)
add_subdirectory(callNumpy)
add_subdirectory(callPyDPI)
# find_package (Python3 EXACT COMPONENTS Interpreter Development REQUIRED)


set(PYTHON_EXECUTABLE "C:/Users/xx/AppData/Local/Programs/Python/Python310/python.exe")


find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)

# add_subdirectory(callCpp)
add_subdirectory(callPy)


# find_package(Python COMPONENTS Interpreter Development)
# find_package(pybind11 CONFIG)

# # pybind11 method:
# pybind11_add_module(MyModule1 src1.cpp)

# # Python method:
# Python_add_library(MyModule2 src2.cpp)
# target_link_libraries(MyModule2 pybind11::headers)
# set_target_properties(MyModule2 PROPERTIES
#     INTERPROCEDURAL_OPTIMIZATION ON
#     CXX_VISIBILITY_PRESET ON
#     VISIBILITY_INLINES_HIDDEN ON
# )