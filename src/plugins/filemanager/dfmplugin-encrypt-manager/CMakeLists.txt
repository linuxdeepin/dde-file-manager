cmake_minimum_required(VERSION 3.10)

project(dfmplugin-encrypt-manager)
set(BIN_NAME dfm-encrypt-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Include DFM plugin configuration module
include(DFMPluginConfig)

file(GLOB_RECURSE SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.json)

add_library(${BIN_NAME} SHARED
    ${SRC})

set_target_properties(${BIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DFM_BUILD_PLUGIN_FILEMANAGER_DIR})

# Use DFM plugin configuration system
dfm_apply_default_plugin_config(${BIN_NAME})

install(TARGETS ${BIN_NAME} LIBRARY DESTINATION ${DFM_PLUGIN_FILEMANAGER_CORE_DIR})
