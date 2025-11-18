# this is heuristically generated, and may not be correct
# find_package(OpenCASCADE CONFIG REQUIRED)
# note: 45 additional targets are not displayed.
# target_link_libraries(main PRIVATE TKBO TKBin TKCAF TKCDF)

# # 对于VTK，避免一些内部警告，使用组件化方法
find_package(VTK REQUIRED COMPONENTS CommonCore RenderingCore)
# include("${VTK_USE_FILE}")
# target_link_libraries(main PRIVATE ${VTK_LIBRARIES})

######################################################

MESSAGE(" --------- OpenCASCADE TestProj----- \n")

# 解决CMP0167政策警告 - FindBoost模块已移除的问题
cmake_policy(SET CMP0167 NEW)

set(CPP_DIR
    ./
)

find_package(OpenCASCADE CONFIG REQUIRED)

if(OpenCASCADE_FOUND)
    # MESSAGE( "--------------OpenCASCADE_FOUND Found")
    MESSAGE( " ---OpenCASCADE_FOUND Include: ${OpenCASCADE_INCLUDE_DIRS}")
    MESSAGE( " ---OpenCASCADE_FOUND Library: ${OpenCASCADE_LIBRARY_DIRS}")

    include_directories(${OpenCASCADE_INCLUDE_DIRS})
    link_directories(${OpenCASCADE_LIBRARY_DIRS})

else()
    MESSAGE(FATAL_ERROR "------OpenCASCADE Not Found")
endif(OpenCASCADE_FOUND)

# Vtk,(visualization toolkit)是一个开源的免费软件系统,主要用于三维计算机图形学、图像处理和可视化.
find_package(VTK REQUIRED)
if(VTK_FOUND)
    # MESSAGE( "--------------VTK_FOUND Found")
    MESSAGE( " ---VTK_FOUND Include: ${VTK_INCLUDE_DIRS}")
    MESSAGE( " ---VTK_FOUND Library: ${VTK_LIBRARY_DIRS}")

    # include("${VTK_USE_FILE}")

    include_directories(${VTK_INCLUDE_DIRS})
    link_directories(${VTK_LIBRARY_DIRS})

else()
    MESSAGE(FATAL_ERROR "------VTK Not Found")
endif(VTK_FOUND)

# 寻找freetype库和包含文件
# OpenCASCADE依赖于freetype库用于一些文本和字体相关的操作
find_package(freetype CONFIG REQUIRED)
if(freetype_FOUND)
    # MESSAGE( "--------------freetype_FOUND Found")
    MESSAGE( " ---freetype_FOUND Include: ${freetype_INCLUDE_DIRS}")
    MESSAGE( " ---freetype_FOUND Library: ${freetype_LIBRARY_DIRS}")

    include_directories(${freetype_INCLUDE_DIRS})
    link_directories(${freetype_LIBRARY_DIRS})

endif(freetype_FOUND)


# if(WIN32)
#     # set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
#     list(APPEND CMAKE_PREFIX_PATH C:/Install/Qt/5.15.2/msvc2019_64/lib/cmake)
# elseif(UNIX AND NOT APPLE)
#     # set(VCPKG_PATH /home/x/install/vcpkg//scripts/buildsystems/vcpkg.cmake)
#     list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
# elseif(APPLE)
# endif(WIN32)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
if(Qt5_FOUND)

else()
    MESSAGE(FATAL_ERROR "------Qt5 Not Found")
endif()


foreach(SUB_DIR ${CPP_DIR})
    file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/*.cpp")
    foreach(CPP ${SRC})
        # message(${SRC})
        # message(${CPP})
        STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${CPP})
        add_executable(OpenCASCADE_${FILE_NAME} ${CPP})

        # target_link_libraries(OpenCASCADE_${FILE_NAME} PRIVATE OpenCASCADE::OpenCASCADE)
        # 避免直接使用${VTK_LIBRARIES}中的弃用库Ioss和Ionit
        # 改为直接链接到OpenCASCADE和freetype库，或根据需要明确指定VTK组件
        target_link_libraries(OpenCASCADE_${FILE_NAME} PRIVATE TKBO TKBin TKCAF TKCDF ${OpenCASCADE_LIBRARIES} ${FREETYPE_LIBRARIES})
        # target_link_libraries(main PRIVATE ${VTK_LIBRARIES})
    endforeach(CPP)

endforeach(SUB_DIR)

