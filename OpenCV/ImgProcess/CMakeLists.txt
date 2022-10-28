######################################################
message(" --------- OpenCV  ImgProcess ----- \n")

#################### Lib ##################################
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/imgProcess.cpp)

add_library(ImgProcess ${SRC_FILES})

target_include_directories(ImgProcess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ImgProcess  PUBLIC ${OpenCV_LIBS})

#################### Exe ##################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

######################################################

set(TS_FILES test/QtOpenCVTest_zh_CN.ts)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test EXE_SRC)

set(PROJECT_SOURCES
        test/main.cpp
        test/mainwindow.cpp
        test/mainwindow.h
        test/mainwindow.ui
        ${TS_FILES}
)

SET(APP_NAME  QtOpenCVTest)

qt_add_executable(${APP_NAME}
        MANUAL_FINALIZATION 
        ${PROJECT_SOURCES}
    )

qt_create_translation(QM_FILES ${EXE_SRC} ${TS_FILES})

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets )

link_directories(${cmake_source_dir}/build/)

#librdImgProcess.a
target_link_libraries(${APP_NAME} PRIVATE ImgProcess)

######################################################
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})

target_link_libraries(ImgProcess  PUBLIC ${OpenCV_LIBS})

######################################################

set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER cv.testQt.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${APP_NAME})
endif()
