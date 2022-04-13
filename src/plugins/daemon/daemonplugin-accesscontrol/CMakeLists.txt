cmake_minimum_required(VERSION 3.10)

project(daemonplugin-accesscontrol)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB FILEOPERATIONS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )


add_library(${PROJECT_NAME}
    SHARED
    ${FILEOPERATIONS_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../)

find_package(PkgConfig REQUIRED)
pkg_search_module(dfm-mount
    REQUIRED
    dfm-mount
    IMPORTED_TARGET
)
pkg_search_module(dfm-io
    REQUIRED
    dfm-io
    IMPORTED_TARGET
)
find_package(Qt5 COMPONENTS
    DBus
    REQUIRED
)

target_link_libraries(${PROJECT_NAME}
    DFM::framework
    PkgConfig::dfm-mount
    PkgConfig::dfm-io
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_DIR}
)
