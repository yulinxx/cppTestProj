message(" --------- machine_status ----- \n")
aux_source_directory(./ SM_SRC)
message(${PROJECT_BINARY_DIR})
add_executable(StateMachine ${SM_SRC})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
