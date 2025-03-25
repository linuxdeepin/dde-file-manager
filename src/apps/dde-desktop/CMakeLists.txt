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

# Qt6
if (DFM_BUILD_WITH_QT6)
    include(dde-desktop-qt6.cmake)
else()
    include(dde-desktop-qt5.cmake)
endif()


