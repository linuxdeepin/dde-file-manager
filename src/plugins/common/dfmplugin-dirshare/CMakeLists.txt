cmake_minimum_required(VERSION 3.10)

project(dfmplugin-dirshare)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE DIRSHARE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

include(dfmplugin-dirshare-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-dirshare-qt6.cmake)
endif()
