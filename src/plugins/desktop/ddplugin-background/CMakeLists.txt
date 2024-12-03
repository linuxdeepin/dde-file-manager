cmake_minimum_required(VERSION 3.10)

project(ddplugin-background)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

#delete files which are not needed on the current system
if (COMPILE_ON_V2X)
    FILE(GLOB_RECURSE RM_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/backgroundwm.cpp"
        )
else()
    FILE(GLOB_RECURSE RM_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/backgrounddde.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/backgrounddde.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/appearance_interface.cpp"
        )
endif()

list(REMOVE_ITEM SRC_FILES ${RM_SRC})


# Qt6
if (DFM_BUILD_WITH_QT6)
    include(ddplugin-background-qt6.cmake)
else()
    include(ddplugin-background-qt5.cmake)
endif()

