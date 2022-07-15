cmake_minimum_required(VERSION 3.10)

project(daemonplugin-tagdatamanager)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB TAGDAEMON_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )


add_library(${PROJECT_NAME}
    SHARED
    ${TAGDAEMON_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(Qt5 COMPONENTS
    DBus
    Sql
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    Qt5::DBus
    Qt5::Sql
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DAEMON_EDGE_DIR}
)
