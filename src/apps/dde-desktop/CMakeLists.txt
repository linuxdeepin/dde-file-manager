cmake_minimum_required(VERSION 3.10)

project(dde-desktop)

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRCS
    main.cpp
    desktopdbusinterface.cpp
    desktopdbusinterface.h
)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Widgets
    DBus
    REQUIRED)

find_package(Dtk COMPONENTS Widget REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5DBus_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    DFM::base
    DFM::framework
    Qt5::Widgets
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# dbus
install(FILES data/com.deepin.dde.desktop.service DESTINATION share/dbus-1/services)

# computer and trash
install(FILES data/applications/dde-computer.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-trash.desktop DESTINATION share/applications/)
install(FILES data/applications/dde-home.desktop DESTINATION share/applications/)

