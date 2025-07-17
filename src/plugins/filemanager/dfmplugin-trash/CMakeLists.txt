cmake_minimum_required(VERSION 3.10)

project(dfmplugin-trash)
set(BIN_NAME dfm-trash-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include DFM plugin configuration module
include(DFMPluginConfig)

FILE(GLOB_RECURSE TRASH_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

add_library(${BIN_NAME}
    SHARED
    ${TRASH_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use DFM plugin configuration system
include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
dfm_setup_trash_dependencies(${BIN_NAME})

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
