cmake_minimum_required(VERSION 3.1.0)

project(dde-file-manager)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
configure_file(
    "config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(SRCS
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
    )

target_link_libraries(
    ${PROJECT_NAME}
    DFM::framework
    ${QtUseModules}
    ${PkgUseModules}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# TODO(zhangs): translation
