cmake_minimum_required(VERSION 3.10)

project(dfmplugin-propertydialog)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


FILE(GLOB_RECURSE DETAILSPCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-propertydialog-qt6.cmake)
endif()

include(dfmplugin-propertydialog-qt5.cmake)



