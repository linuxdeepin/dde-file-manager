cmake_minimum_required(VERSION 3.10)

project(daemonplugin-core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

find_package(PkgConfig)
pkg_check_modules(PolkitAgent REQUIRED polkit-agent-1)
pkg_check_modules(PolkitQt5 REQUIRED polkit-qt5-1)

add_library(${PROJECT_NAME}
    SHARED
    ${SRC_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_DAEMON_DIR})

find_package(Qt5 COMPONENTS Core)
target_link_libraries(${PROJECT_NAME}
    DFM::base
    DFM::framework
    ${DtkWidget_LIBRARIES}
    ${PolkitAgent_LIBRARIES}
    ${PolkitQt5_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
