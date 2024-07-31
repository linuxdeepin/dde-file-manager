cmake_minimum_required(VERSION 3.10)

project(dfmplugin-emblem)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE GLOB EMBLEM_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

include(dfmplugin-emblem-qt5.cmake)

# Qt6
if (DFM_BUILD_WITH_QT6)
include(dfmplugin-emblem-qt6.cmake)
endif()

# install dconfig files.
INSTALL_DCONFIG("org.deepin.dde.file-manager.emblem.json")
