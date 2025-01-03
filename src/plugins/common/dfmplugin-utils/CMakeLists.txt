cmake_minimum_required(VERSION 3.10)

project(dfmplugin-utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
    "${APP_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

FILE(GLOB_RECURSE UTILS_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

# NOTE: maybe a Qt bug, qt6 before qt5 if use QtSvg
# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-utils-qt6.cmake)
endif()

include(dfmplugin-utils-qt5.cmake)


