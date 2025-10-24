cmake_minimum_required(VERSION 3.10)

project(ds-panel-desktop )

configure_file(
    "${DFM_APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

find_package(DDEShell REQUIRED)
find_package(Qt6 COMPONENTS DBus REQUIRED)

FILE(GLOB_RECURSE FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(ds-panel-desktop SHARED
    ${FILES}
)

target_compile_definitions(${PROJECT_NAME}
 PRIVATE
     DDE_LAUNCHPAD_VERSION=${CMAKE_PROJECT_VERSION}
 )

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${Qt6Widgets_PRIVATE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Dde::Shell
    DFM6::base
    DFM6::framework
    Qt6::DBus
)

ds_install_package(PACKAGE org.deepin.ds.desktop TARGET ds-panel-desktop)

# dbus
install(FILES data/com.deepin.dde.desktop.service DESTINATION share/dbus-1/services)

# computer and trash
install(FILES data/applications/dfm-trash.sh DESTINATION bin/)
install(FILES data/applications/dde-computer.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-trash.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-home.desktop DESTINATION share/applications/)
