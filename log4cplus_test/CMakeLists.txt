message(" --------- log4cplus_test ----- \n")

find_package(log4cplus CONFIG REQUIRED)

aux_source_directory(. DIR_SRCS)

add_executable(rdLog ${DIR_SRCS})

target_include_directories(rdLog PRIVATE .)

target_link_libraries(rdLog PRIVATE log4cplus::log4cplus)