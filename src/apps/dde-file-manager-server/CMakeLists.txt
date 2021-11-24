cmake_minimum_required(VERSION 3.1.0)

project(dde-file-manager-server)

configure_file(
    "config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

set(SERVER_SRCS
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SERVER_SRCS})

target_include_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_BINARY_DIR}
    )

target_link_libraries(
    ${PROJECT_NAME}
    DFM::framework
    ${QtUseModules}
    ${PkgUseModules}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

