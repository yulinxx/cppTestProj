cmake_minimum_required(VERSION 3.10)

project(LibUnitTest)
set(UTest_NAME LibUnitTest)

# 添加测试目标  
enable_testing()

add_executable(${UTest_NAME} ./unitTest/ImplementationTest.cpp) 

# 查找 gtest 和 gmock（如果你使用 gmock）  
find_package(GTest CONFIG REQUIRED)
target_link_libraries(${UTest_NAME} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(AllTestsInMain ${UTest_NAME})

# 添加测试到 CTest  
add_test(NAME MyTest COMMAND ${UTest_NAME})

target_include_directories(${UTest_NAME} PUBLIC ../)
target_include_directories(${UTest_NAME} PUBLIC ../LibExportInterface/)

target_link_libraries(${UTest_NAME} PUBLIC LibExportImplementation)