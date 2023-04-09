
MESSAGE(" ---------  ThreadPool ----- \n")


# set(PROJECT_SOURCES
#         main.cpp
#         xxThreadPool.cpp


SET(APP_NAME  threadPool)

# add_executable(${APP_NAME}
#         ${PROJECT_SOURCES}
#     )

# aux_source_directory(${APP_NAME} DIR_SRCS)

add_executable(${APP_NAME} main.cpp xxThreadPool.cpp)

if(MSVC)
    target_compile_options(${APP_NAME} PUBLIC "/Zc:__cplusplus")
endif()

