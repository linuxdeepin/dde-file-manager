cmake_minimum_required(VERSION 3.10)

project(dfmplugin-myshares)
set(BIN_NAME dfm-myshares-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include DFM plugin configuration module
include(DFMPluginConfig)

FILE(GLOB_RECURSE SHARES_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
    )

add_library(${BIN_NAME}
    SHARED
    ${SHARES_FILES}
)

add_library(DFMPlugin::myshares ALIAS ${BIN_NAME})

set_target_properties(${BIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use DFM plugin configuration system
dfm_apply_default_plugin_config(${BIN_NAME})

#install library file
install(TARGETS
    ${BIN_NAME}
    LIBRARY
    DESTINATION
    ${DFM_PLUGIN_FILEMANAGER_CORE_DIR}
)
