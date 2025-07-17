cmake_minimum_required(VERSION 3.10)

project(dfmplugin-core)
set(BIN_NAME dfm-core-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include plugin configuration
include(DFMPluginConfig)

FILE(GLOB_RECURSE CORE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${BIN_NAME}
    SHARED
    ${CORE_FILES}
)

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use default plugin configuration
dfm_apply_default_plugin_config(${BIN_NAME})

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
