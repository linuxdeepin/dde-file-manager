cmake_minimum_required(VERSION 3.10)

project(dde-file-manager-server)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

set(SRCS
    main.cpp
)

find_package(Qt5 COMPONENTS
    Core
    DBus
    REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::Core
    Qt5::DBus
)

# binary
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# services
install(FILES dbusservice/org.deepin.filemanager.server.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services)

# systemd service
install(FILES dbusservice/dde-filemanager-server.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
