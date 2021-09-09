cmake_minimum_required(VERSION 3.1.0)

project(dde-file-manager-server)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    )

set(SERVER_SRCS
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SERVER_SRCS})

target_link_libraries(
    ${PROJECT_NAME}
    dfm-framework
    ${QtUseModules}
    ${PkgUseModules}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${RUNTIME_INSTALL_RPEFIX})

# TODO(zhangs): translation
