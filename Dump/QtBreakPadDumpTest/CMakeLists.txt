cmake_minimum_required(VERSION 3.10)

# 使用vcpkg进行包管理
if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
endif(WIN32)

set(CMAKE_TOOLCHAIN_FILE  ${VCPKG_PATH})

#
SET(APP_NAME QtBreakPadDumpTest)
project(${APP_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        QBreakpad.h
        QBreakpad.cpp
        QBreakpadPrivate.cpp
        QBreakpadPrivate.h
        WindowsDllInterceptor.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${APP_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${APP_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${APP_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${APP_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${APP_NAME})
endif()

MESSAGE(" --------- breakpad ----- \n")

target_include_directories(${APP_NAME} PRIVATE ./)
find_package(unofficial-breakpad CONFIG REQUIRED)
target_link_libraries(${APP_NAME}  PRIVATE unofficial::breakpad::libbreakpad unofficial::breakpad::libbreakpad_client)

#if(unofficial-breakpad_FOUND)

#else()
#    MESSAGE(FATAL_ERROR "------breakpad Not Found")
#endif()


