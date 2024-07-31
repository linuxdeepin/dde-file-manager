cmake_minimum_required(VERSION 3.10)

project(dfmplugin-menu)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE MENU_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

include(dfmplugin-menu-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-menu-qt6.cmake)
endif()
