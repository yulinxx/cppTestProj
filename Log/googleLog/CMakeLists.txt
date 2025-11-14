################# GLOG #####################################
MESSAGE(" --------- google glog ----- \n")

find_package(glog CONFIG REQUIRED)
if(glog_FOUND)
    MESSAGE( " -----glog_FOUND")
    # target_link_libraries(${APP_NAME} PRIVATE glog::glog)
    # target_link_libraries(${APP_NAME} PRIVATE ${glog_LIBRARIES})
    target_link_libraries(${APP_NAME}  glog::glog)
else()
    MESSAGE(FATAL_ERROR "------glog Not Found")
endif()

aux_source_directory(. DIR_SRCS)

add_executable(googleGLOG_gLog ${DIR_SRCS})

target_include_directories(googleGLOG_gLog PRIVATE .)

target_link_libraries(googleGLOG_gLog PRIVATE  glog::glog)