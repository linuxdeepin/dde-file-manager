cmake_minimum_required(VERSION 3.10)

project(dde-file-manager-daemon)

message("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw")
    add_definitions(-DDISABLE_ANYTHING)
endif()

set(SRCS
    main.cpp
)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Widgets
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
    Qt5::Widgets
    Qt5::DBus
)

# binary
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# systembusconf
install(FILES dbusservice/com.deepin.filemanager.daemon.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d)

# services
install(FILES dbusservice/com.deepin.filemanager.daemon.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/system-services)

# systemd service
install(FILES dbusservice/dde-filemanager-daemon.service
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system)
