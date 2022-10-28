MESSAGE(" --------- log4cplus_test ----- \n")

find_package(log4cplus CONFIG REQUIRED)
if(log4cplus_FOUND)

elseif()
    MESSAGE(FATAL_ERROR "------log4cplus Not Found")
endif()

aux_source_directory(. DIR_SRCS)

SET(APP_NAME  Llog4CPlusTest)
add_executable(${APP_NAME} ${DIR_SRCS})

target_include_directories(${APP_NAME} PRIVATE .)

target_link_libraries(${APP_NAME} PRIVATE log4cplus::log4cplus)