MESSAGE(" --------- skia TestProj----- \n")


######################################################
if(WIN32)
    # set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
    list(APPEND CMAKE_PREFIX_PATH C:/Install/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    # set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
endif(WIN32)

list(APPEND CMAKE_PREFIX_PATH C:/Install/Qt/5.15.2/msvc2019_64/lib/cmake)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS  Core Gui Widgets REQUIRED)

if(Qt5_FOUND)

else()
    MESSAGE(FATAL_ERROR "------Qt5 Not Found")
endif()

#################### Exe ##################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

######################################################
# find_package(skia CONFIG REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)

if(unofficial-skia_FOUND)
    MESSAGE( "--------------unofficial-skia_FOUND Found")
    MESSAGE( " ---unofficial-skia_FOUND Include: ${unofficial-skia_INCLUDE_DIRS}")
    MESSAGE( " ---unofficial-skia_FOUND Library: ${unofficial-skia_LIBRARY_DIRS}")

    include_directories(${unofficial-skia_INCLUDE_DIRS})
    link_directories(${unofficial-skia_LIBRARY_DIRS})
else()
    MESSAGE(FATAL_ERROR "------unofficial-skia Not Found")
endif(unofficial-skia_FOUND)

######################################################

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ EXE_SRC)

set(PROJECT_SOURCES
        main.cpp
        SkiaCircleWidget.cpp
        SkiaCircleWidget.h
        # test/mainwindow.ui
        # ${TS_FILES}
)

SET(APP_NAME  SkiaTestProj)

add_executable(${APP_NAME}
        ${PROJECT_SOURCES}
    )

# qt_add_executable(${APP_NAME}
#         MANUAL_FINALIZATION 
#         ${PROJECT_SOURCES}
#     )

qt_create_translation(QM_FILES ${EXE_SRC} ${TS_FILES})

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

target_link_libraries(${APP_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets )

target_link_libraries(${APP_NAME} PRIVATE unofficial::skia::skia)


######################################################

# set_target_properties(${APP_NAME} PROPERTIES
#     MACOSX_BUNDLE_GUI_IDENTIFIER cv.testQt.com
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
# )