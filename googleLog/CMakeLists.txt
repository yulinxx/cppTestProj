################# GLOG #####################################
message(" --------- google glog ----- \n")

find_package(glog CONFIG REQUIRED)
if(glog_FOUND)
    message(STATUS  " -----glog_FOUND")
    # target_link_libraries(${APP_NAME} PRIVATE glog::glog)
    # target_link_libraries(${APP_NAME} PRIVATE ${glog_LIBRARIES})
    target_link_libraries(${APP_NAME}  glog::glog)
endif()

aux_source_directory(. DIR_SRCS)

add_executable(googleGLOG_gLog ${DIR_SRCS})

target_include_directories(googleGLOG_gLog PRIVATE .)

target_link_libraries(googleGLOG_gLog PRIVATE  glog::glog)